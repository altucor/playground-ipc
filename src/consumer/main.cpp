#include "common/Pipe.hpp"
#include "common/Client.hpp"
#include "common/Packet.hpp"
#include "common/Utils.hpp"

#include <print>
#include <span>
#include <cstddef>
#include <vector>
#include <thread>
#include <chrono>
#include <array>
#include <deque>
#include <mutex>

/*

Consumer:
 - 1) Receive packets from other process
 - 2) Validate checksum and appended metadata
 - 3) Once per second print stats: total packets, packets per second, bytes per second
 - 4) Can pause and resume transfer by handling signals and by pressing any key, at the moment of pause, data should be
ignored

*/

class Session
{
public:
    Session(std::stop_source& stopSource)
        : m_stopSource(stopSource)
        , m_client(stopSource)
    {
    }

    ~Session()
    {
        m_stopSource.request_stop();
        if (m_receiver.joinable())
        {
            m_receiver.join();
        }

        if (m_processor.joinable())
        {
            m_processor.join();
        }

        if (m_performanceMonitor.joinable())
        {
            m_performanceMonitor.join();
        }

        std::print("[Session] Absolute maximum packets per second: {:d}\n", m_packetsPerSecond.getAbsoluteMaximum());
        std::print(
            "[Session] Absolute maximum bytes per second: {:d}\n", m_client.getRecvPerfCounter().getAbsoluteMaximum());
        std::print("[Session] Total packet receive(valid): {:d}\n", m_packetsReceivedTotal);
    }

    int start()
    {
        if (m_client.start() != 0)
        {
            std::print("[Session] Failed to start client\n");
            return -1;
        }

        m_receiver = std::thread(
            [&]()
            {
                std::print("[Session] Receiver thread started\n");

                while (!m_stopSource.stop_requested())
                {
                    auto packet = common::Packet();
                    packet.unmarshal(m_client);
                    packet.debug();

                    if (packet.valid())
                    {
                        std::lock_guard<std::mutex> lock(m_mutexPacketsQueue);
                        m_packets.push_back(std::move(packet));
                        m_packetsPerSecond.update();
                        m_packetsReceivedTotal++;
                    }
                    else
                    {
                        std::print("[Session] Warning got invalid packet, discarding\n");
                    }
                }

                std::print("[Session] Receiver thread finished\n");
            });

        m_processor = std::thread(
            [&]()
            {
                std::print("[Session] Packet processor thread started\n");

                while (!m_stopSource.stop_requested())
                {
                    std::lock_guard<std::mutex> lock(m_mutexPacketsQueue);
                    if (m_packets.empty())
                    {
                        continue;
                    }

                    auto packet = std::move(m_packets.front());
                    m_packets.pop_front();

                    if (packet.getTime() > m_lastTimestamp &&
                        common::isNewer(packet.getSequenceNumber(), m_lastSequenceNumber))
                    {
                        // Only if all is okay then update last seen sequence number
                        m_lastTimestamp = packet.getTime();
                        m_lastSequenceNumber = packet.getSequenceNumber();
                    }
                    else
                    {
                        if (packet.getSequenceNumber() == m_lastSequenceNumber)
                        {
                            std::print(
                                "[Session] Worker got duplicate packet under index: {:d}(0x{:02X})\n",
                                m_lastSequenceNumber,
                                m_lastSequenceNumber);
                        }
                        else if (!common::isNewer(packet.getSequenceNumber(), m_lastSequenceNumber))
                        {
                            std::print(
                                "[Session] Worker got old packet under index: {:d}(0x{:02X})\n",
                                packet.getSequenceNumber(),
                                packet.getSequenceNumber());
                        }

                        if (packet.getTime() == m_lastTimestamp)
                        {
                            std::print(
                                "[Session] Worker got duplicate packet with same tmestamp: {:s}\n",
                                std::format("{:%Y-%m-%d %H:%M:%S}", m_lastTimestamp));
                        }
                        else if (packet.getTime() <= m_lastTimestamp)
                        {
                            std::print(
                                "[Session] Worker got old packet with timestamp from before: {:s}, last good timestamp "
                                "is: {:s}\n",
                                std::format("{:%Y-%m-%d %H:%M:%S}", packet.getTime()),
                                std::format("{:%Y-%m-%d %H:%M:%S}", m_lastTimestamp));
                        }
                    }
                }

                std::print("[Session] Packet processor thread finished\n");
            });

        m_performanceMonitor = std::thread(
            [&]()
            {
                std::print("[Session] Performance monitor thread started\n");

                while (!m_stopSource.stop_requested())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    std::print("[Session] Monitor:: Packets per second: {:d}\n", m_packetsPerSecond.get());
                    std::print("[Session] Monitor:: Bytes per second: {:d}\n", m_client.getRecvPerfCounter().get());
                    std::print("[Session] Monitor:: Packet receive total(valid): {:d}\n", m_packetsReceivedTotal);
                }

                std::print("[Session] Performance monitor thread finished\n");
            });

        return 0;
    }

private:
    std::stop_source& m_stopSource;
    std::chrono::system_clock::time_point m_lastTimestamp;
    std::size_t m_lastSequenceNumber = 0;
    common::Client m_client;
    std::thread m_receiver;
    std::thread m_processor;
    std::thread m_performanceMonitor;
    mutable std::mutex m_mutexPacketsQueue;
    std::deque<common::Packet> m_packets;

    // Metrics related stuff
    std::size_t m_packetsReceivedTotal = 0;
    common::PerformanceCounter m_packetsPerSecond;

    std::size_t m_packetsPerSecondAbsolute = 0;
    std::size_t m_bytesPerSecondAbsolute = 0;
};

int main(int argc, char* argv[])
{
    std::print("[Consumer] Started\n");

    std::stop_source stopSource;

    auto session = Session(stopSource);

    int result = session.start();
    if (result == -1)
    {
        std::print("[Consumer] Failed to start client session\n");
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    stopSource.request_stop();

    std::print("[Consumer] Finished\n");

    return 0;
}
