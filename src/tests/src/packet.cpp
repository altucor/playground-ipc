#include <gtest/gtest.h>

#include "common/Utils.hpp"
#include "common/Packet.hpp"

#include <vector>
#include <span>
#include <chrono>
#include <algorithm>

using namespace common;

TEST(packet, marshal_and_unmarshal)
{
    constexpr std::size_t kPayloadSize = 1000;

    auto packetFirst = common::Packet(kPayloadSize, 1337);
    EXPECT_TRUE(packetFirst.valid());

    auto blob = packetFirst.marshal();

    constexpr std::size_t kExpectedBlobSize =
        kPayloadSize + sizeof(uint64_t) + sizeof(std::size_t) + sizeof(std::size_t) + sizeof(std::byte);
    EXPECT_EQ(blob.size(), kExpectedBlobSize);

    auto packetSecond = common::Packet(kPayloadSize, blob);
    packetSecond.debug();

    EXPECT_EQ(packetFirst.getTime(), packetSecond.getTime());
    EXPECT_EQ(packetFirst.getSequenceNumber(), packetSecond.getSequenceNumber());
    EXPECT_EQ(packetFirst.getPayload(), packetSecond.getPayload());
    EXPECT_EQ(packetFirst.getChecksum(), packetSecond.getChecksum());
}
