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

class Session
{
public:
    explicit Session(std::stop_source& stopSource, const std::size_t payloadSize)
        : m_stopSource(stopSource)
        , m_payloadSize(payloadSize)
    {
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

        m_worker = std::jthread(
            [&](std::stop_token stopToken)
            {
                while (!stopToken.stop_requested())
                {
                    auto packet = common::Packet(m_payloadSize, m_sequenceIndex);
                    packet.debug();
                    auto packetData = packet.marshal();
                    if (m_server.write(packetData, stopToken) != packetData.size())
                    {
                        std::print("[Session] Error sending packet\n");
                    }

                    m_sequenceIndex++;

                    // TODO: remove sleep. Right now used only to limit debug message spamming
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            });

        return result;
    }

private:
    std::stop_source& m_stopSource;
    const std::size_t m_payloadSize = 0;
    std::size_t m_sequenceIndex = 0;
    common::Server m_server;
    std::jthread m_worker;
};

int main(int argc, char* argv[])
{
    std::print("[Producer] Started\n");

    std::stop_source stopSource;

    constexpr std::size_t kInitialSize = 100;
    std::size_t payloadSize = kInitialSize;

    if (argc == 2)
    {
        payloadSize = std::atoll(argv[1]);
    }

    std::print("Payload size set to: {:d}\n", payloadSize);

    auto session = Session(stopSource, payloadSize);
    int result = session.start(std::chrono::milliseconds(3000));
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
