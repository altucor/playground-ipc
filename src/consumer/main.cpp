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

        m_worker = std::jthread(
            [&](std::stop_token stopToken)
            {
                while (!stopToken.stop_requested())
                {
                    auto packet = common::Packet();
                    packet.unmarshal(m_client);
                    packet.debug();
                    std::print("Packet valid: {:s}\n", (packet.valid() ? "YES" : "NO"));
                    std::print(" ---------------------------------- \n");
                }
            });

        return 0;
    }

private:
    std::size_t m_sequenceIndex = 0;
    common::Client m_client;
    std::jthread m_worker;
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
