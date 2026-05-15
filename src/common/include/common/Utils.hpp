#pragma once

#include <cstdint>

namespace common
{
    template <std::size_t N>
    static const auto makeBytes(const uint8_t (&arr)[N])
    {
        return std::as_bytes(std::span<const uint8_t>(arr));
    }
}
