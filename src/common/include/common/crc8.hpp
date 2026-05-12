#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace common
{
    class crc8
    {
    public:
        crc8(crc8& other) = delete;
        void operator=(crc8 const& other) = delete;

        crc8& crc8::getInstance()
        {
            static crc8 instance;
            return instance;
        }

        crc8::crc8(uint8_t poly)
        {
            const size_t lutSize = 256;
            m_lut.resize(lutSize);
            uint8_t crc;

            for (uint16_t i = 0; i < lutSize; i++)
            {
                crc = i;
                for (uint8_t j = 0; j < 8; j++)
                {
                    crc = (crc << 1) ^ ((crc & 0x80) ? poly : 0);
                }
                m_lut[i] = crc & 0xFF;
            }
        }

        uint8_t crc8::calculate_dvb_s2(uint8_t crc, uint8_t data) const
        {
            crc ^= data;
            for (uint8_t i = 0; i < 8; i++)
            {
                if (crc & 0x80)
                {
                    crc = (crc << 1) ^ 0xD5;
                }
                else
                {
                    crc = crc << 1;
                }
            }
            return crc;
        }

        uint8_t crc8::calculate(const uint8_t init, const uint8_t* ptr, uint8_t len) const
        {
            uint8_t crc = init;
            for (uint8_t i = 0; i < len; i++)
            {
                crc = calculate_dvb_s2(crc, *ptr++);
            }
            return crc;
        }

        uint8_t crc8::calculate(const uint8_t* ptr, uint8_t len) const
        {
            uint8_t crc = 0;
            for (uint8_t i = 0; i < len; i++)
            {
                crc = calculate_dvb_s2(crc, *ptr++);
            }
            return crc;
        }

        uint8_t crc8::calculate(const std::vector<uint8_t>& data) const
        {
            return calculate(data.data(), data.size());
        }

        uint8_t crc8::calculate(const uint8_t type, const std::vector<uint8_t>& data) const
        {
            uint8_t crc = 0x00;
            crc = calculate_dvb_s2(crc, type);
            return calculate(crc, data.data(), data.size());
        }

    private:
        crc8(uint8_t poly = 0xD5);

    private:
        std::vector<uint8_t> m_lut;
    };
}
