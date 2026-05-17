#include <gtest/gtest.h>

#include "common/Utils.hpp"

#include <limits>

using namespace common;

TEST(utils, isNewer)
{
    EXPECT_TRUE(common::isNewer(1, 0));
    EXPECT_FALSE(common::isNewer(0, 1));

    EXPECT_TRUE(common::isNewer(10000, 0));

    // Overflow
    EXPECT_TRUE(common::isNewer(0, std::numeric_limits<uint64_t>::max()));

    // Should false missing N between values "N-1" and "0"
    EXPECT_FALSE(common::isNewer(std::numeric_limits<uint64_t>::max() - 1, 0));
}
