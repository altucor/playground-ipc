#include <gtest/gtest.h>

#include "common/Utils.hpp"
#include "common/Pipe.hpp"
#include "common/Server.hpp"
#include "common/Client.hpp"

#include <vector>
#include <span>
#include <chrono>
#include <algorithm>

using namespace common;

TEST(pipe, server_to_client_basic_transfer)
{
    auto server = Server();
    auto client = Client();

    EXPECT_EQ(server.start(), 0);

    // Let server actually start
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // NOTE: Yes sleeps is bad but for simplicity of tests we use them here.

    EXPECT_EQ(client.start(), 0);

    // Let client establish connection and kernel notify us
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    EXPECT_TRUE(server.clientConnected());

    auto bufferToWrite = makeBytes({0x01, 0x02, 0x03, 0xFF, 0x05, 0x06, 0x07, 0x08});
    EXPECT_EQ(server.write(bufferToWrite), bufferToWrite.size());

    std::vector<std::byte> readBuffer(bufferToWrite.size());
    EXPECT_EQ(client.read(readBuffer), bufferToWrite.size());

    EXPECT_TRUE(std::ranges::equal(bufferToWrite, readBuffer));
}
