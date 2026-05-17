#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

namespace common
{
    class crc8
    {
    public:
        crc8(crc8& other) = delete;
        void operator=(crc8 const& other) = delete;

        static crc8& getInstance()
        {
            static crc8 instance;
            return instance;
        }

        std::byte calculate(const std::span<const std::byte> data, std::byte initial = std::byte(0x00)) const
        {
            auto crc = initial;
            for (const auto& item : data)
            {
                crc = m_lut.at(static_cast<uint8_t>(crc ^ item));
            }
            return crc;
        }

        template <typename... Args>
        std::byte calculate(const Args&... args) const
        {
            auto crc = std::byte(0);
            (..., ([&](const auto& arg) { crc = calculate(std::span<const std::byte>(arg), crc); }(args)));
            return crc;
        }

    private:
        crc8(const std::byte poly = std::byte(0xD5))
        {
            uint8_t crc;

            for (uint16_t i = 0; i < kLutSize; i++)
            {
                crc = i;
                for (uint8_t j = 0; j < 8; j++)
                {
                    crc = (crc << 1) ^ ((crc & 0x80) ? static_cast<uint8_t>(poly) : 0);
                }
                m_lut[i] = std::byte(crc & 0xFF);
            }
        }

    private:
        static const std::size_t kLutSize = 256;
        std::array<std::byte, kLutSize> m_lut;
    };
}
