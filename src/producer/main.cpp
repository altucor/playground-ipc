#include "common/Pipe.hpp"
#include "common/Server.hpp"
#include "common/Packet.hpp"

#include <print>
#include <span>
#include <cstddef>
#include <vector>
#include <thread>
#include <chrono>
#include <array>
#include <thread>
#include <stop_token>

/*

Producer:
 - 1) Can read payload from args
 - 2) Produce packets of custom protocol
 - 3) Add timestamp, seq num, checksumm
 - 4) Transfer via IPC
 - 5) Can pause an resume transfer by keypress of any button or by signal

*/

static bool g_DebugPrintPacket = false;

class Session
{
public:
    explicit Session(std::stop_source& stopSource, const std::size_t payloadSize)
        : m_stopSource(stopSource)
        , m_payloadSize(payloadSize)
        , m_server(stopSource)
    {
    }

    ~Session()
    {
        m_stopSource.request_stop();
        if (m_worker.joinable())
        {
            m_worker.join();
        }

        m_server.stop();
    }

    int start(std::chrono::milliseconds timeout)
    {
        int result = 0;
        result = m_server.start();
        if (result == -1)
        {
            std::print("[Session] Failed to start server\n");
            return -1;
        }

        const auto startTime = std::chrono::steady_clock::now();
        while (!m_server.clientConnected() && !m_stopSource.stop_requested() &&
               std::chrono::steady_clock::now() - startTime < timeout)
        {
        }

        if (!m_server.clientConnected())
        {
            std::print("[Session] Waiting for client timed out\n");
            return -2;
        }

        m_worker = std::thread(
            [&]()
            {
                while (!m_stopSource.stop_requested())
                {
                    auto packet = common::Packet(m_payloadSize, m_sequenceIndex);

                    if (g_DebugPrintPacket)
                    {
                        packet.debug();
                    }

                    auto packetData = packet.marshal();
                    if (m_server.write(packetData) != packetData.size())
                    {
                        std::print("[Session] Error sending packet\n");
                    }

                    m_sequenceIndex++;
                }
            });

        return result;
    }

private:
    std::stop_source& m_stopSource;
    const std::size_t m_payloadSize = 0;
    // We need to set sequence index below to "1" just
    // because index "0" will be treat as duplicate on consumer side
    // So all sending will start from index "1"
    std::size_t m_sequenceIndex = 1;
    common::Server m_server;
    std::thread m_worker;
};

int main(int argc, char* argv[])
{
    std::print("[Producer] Started\n");

    std::stop_source stopSource;

    constexpr std::size_t kInitialSize = 100;
    std::size_t payloadSize = kInitialSize;

    if (argc >= 2)
    {
        payloadSize = std::atoll(argv[1]);

        if (argc == 3)
        {
            if (std::string_view(argv[2]) == "true")
            {
                g_DebugPrintPacket = true;
            }
        }
    }

    std::print("[Producer] Payload size set to: {:d}\n", payloadSize);

    auto session = Session(stopSource, payloadSize);
    int result = session.start(std::chrono::milliseconds(5000));
    if (result != 0)
    {
        std::print("[Producer] Failed to start server session\n");
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    stopSource.request_stop();

    std::print("[Producer] Finished\n");

    return 0;
}
