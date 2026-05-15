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

/*

Producer:
 - 1) Can read payload from args
 - 2) Produce packets of custom protocol
 - 3) Add timestamp, seq num, checksumm
 - 4) Transfer via IPC
 - 5) Can pause an resume transfer by keypress of any button or by signal

*/

int main(int argc, char* argv[])
{
    std::print("[Producer] Started\n");

    testPacket();

    constexpr std::size_t kInitialSize = 100;
    std::size_t payloadSize = kInitialSize;

    if (argc == 2)
    {
        payloadSize = std::atoll(argv[1]);
    }

    std::print("Payload size set to: {:d}\n", payloadSize);

    auto server = common::Server();

    int result = 0;
    result = server.start();
    if (result == -1)
    {
        std::print("[Producer] Failed to start server\n");
        return -1;
    }

    constexpr auto timeout = std::chrono::milliseconds(3000);
    const auto startTime = std::chrono::steady_clock::now();
    while (!server.clientConnected() && std::chrono::steady_clock::now() - startTime < timeout)
    {
    }

    if (!server.clientConnected())
    {
        std::print("[Producer] Waiting for client timed out\n");
        return -1;
    }

    std::size_t total = 0;

    total = server.write(std::as_bytes(std::span<const uint8_t>{std::to_array<uint8_t>({0xAA, 0xBB, 0xCC, 0xDD})}));
    std::print("Written: {:d}\n", total);
    total = 0;

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    std::print("[Producer] Finished\n");

    return 0;
}
