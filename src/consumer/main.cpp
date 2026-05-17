#include "common/Pipe.hpp"
#include "common/Client.hpp"
#include "common/Packet.hpp"

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
    int start()
    {
        if (m_client.start() != 0)
        {
            std::print("[Consumer] Failed to start client\n");
            return -1;
        }

        m_receiver = std::jthread(
            [&](std::stop_token stopToken)
            {
                std::print("[Session] Receiver thread started\n");

                while (!stopToken.stop_requested())
                {
                    auto packet = common::Packet();
                    packet.unmarshal(m_client);
                    packet.debug();
                    std::print("Packet valid: {:s}\n", (packet.valid() ? "YES" : "NO"));
                    if (packet.valid())
                    {
                        std::lock_guard<std::mutex> lock(m_mutexPacketsQueue);
                        m_packets.push_back(std::move(packet));
                    }
                }

                std::print("[Session] Receiver thread finished\n");
            });

        m_processor = std::jthread(
            [&](std::stop_token stopToken)
            {
                while (!stopToken.stop_requested())
                {
                    std::lock_guard<std::mutex> lock(m_mutexPacketsQueue);
                    if (m_packets.empty())
                    {
                        continue;
                    }

                    auto packet = std::move(m_packets.front());
                    m_packets.pop_front();

                    if (packet.getTime() > m_lastTimestamp && packet.getSequenceNumber() > m_lastSequenceNumber)
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
                                "[Session] Worker got duplicate packet under index: {:d}(0x{:02X})",
                                m_lastSequenceNumber,
                                m_lastSequenceNumber);
                        }
                        else if (packet.getSequenceNumber() < m_lastSequenceNumber)
                        {
                            std::print(
                                "[Session] Worker got old packet under index: {:d}(0x{:02X})",
                                packet.getSequenceNumber(),
                                packet.getSequenceNumber());
                        }

                        if (packet.getTime() == m_lastTimestamp)
                        {
                            std::print(
                                "[Session] Worker got duplicate packet with same tmestamp: {:s}",
                                std::format("{:%Y-%m-%d %H:%M:%S}", m_lastTimestamp));
                        }
                        else if (packet.getTime() <= m_lastTimestamp)
                        {
                            std::print(
                                "[Session] Worker got old packet with timestamp from before: {:s}, last good timestamp "
                                "is: {:s}",
                                std::format("{:%Y-%m-%d %H:%M:%S}", packet.getTime()),
                                std::format("{:%Y-%m-%d %H:%M:%S}", m_lastTimestamp));
                        }
                    }
                }
            });

        return 0;
    }

private:
    std::chrono::system_clock::time_point m_lastTimestamp;
    std::size_t m_lastSequenceNumber = 0;
    common::Client m_client;
    std::jthread m_receiver;
    std::jthread m_processor;
    mutable std::mutex m_mutexPacketsQueue;
    std::deque<common::Packet> m_packets;
};

int main(int argc, char* argv[])
{
    std::print("[Consumer] Started\n");

    std::stop_source stopSource;

    auto session = Session();

    int result = session.start();
    if (result == -1)
    {
        std::print("[Consumer] Failed to start client session\n");
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    std::print("[Consumer] Finished\n");

    return 0;
}
