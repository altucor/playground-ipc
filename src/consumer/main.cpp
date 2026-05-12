#include "common/Pipe.hpp"
#include "common/Client.hpp"

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

int main(int argc, char* argv[])
{
    std::print("[Consumer] Started\n");

    auto client = common::Client();

    int result = 0;
    result = client.start();
    if (result == -1)
    {
        std::print("[Producer] Failed to start client\n");
        return -1;
    }

    std::size_t total = 0;

    std::vector<std::byte> readBuffer(4);
    total = client.read(readBuffer);
    std::print("Read: {:d}\n", total);
    total = 0;

    for (const auto& item : readBuffer)
    {
        std::print("Result: 0x{:X}\n", static_cast<uint8_t>(item));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    std::print("[Consumer] Finished\n");

    return 0;
}
