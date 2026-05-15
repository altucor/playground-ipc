#include <gtest/gtest.h>

#include "common/Utils.hpp"
#include "common/crc8.hpp"

#include <span>

using namespace common;

TEST(crc8, crc8_calculate)
{
    EXPECT_EQ(std::byte(0x75), crc8::getInstance().calculate(makeBytes({0x01, 0x02, 0x03, 0x04})));
    EXPECT_EQ(std::byte(0x1F), crc8::getInstance().calculate(makeBytes({0xA3, 0xB4, 0xC5, 0xD6})));
    EXPECT_EQ(std::byte(0x3D), crc8::getInstance().calculate(makeBytes({0xF0, 0x0F, 0x11, 0x13})));
    EXPECT_EQ(std::byte(0x5B), crc8::getInstance().calculate(makeBytes({0x75, 0x38, 0xA4, 0x9F})));
}
