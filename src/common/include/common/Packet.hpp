#pragma once

#include "common/crc8.hpp"

#include <chrono>
#include <cstddef>
#include <vector>
#include <span>
#include <ranges>

namespace common
{
    static void fillRandomData(std::vector<std::byte>& data, const std::size_t size)
    {
        if (size < sizeof(int))
        {
            return;
        }

        data.resize(size);

        // Optimized fill with 4 bytes per step
        for (std::size_t i = 0; i < data.size() / sizeof(int); i++)
        {
            if (data.size() - i < sizeof(int))
            {
                for (std::size_t j = 0; j < data.size() - i; j++)
                {
                    data[j] = static_cast<std::byte>(rand());
                }
            }
            else
            {
                auto dst = reinterpret_cast<int*>(&data[i * sizeof(int)]);
                *dst = rand();
            }
        }
    }

    class Packet
    {
    public:
        Packet(const std::size_t payloadSize)
        {
            m_payload.resize(payloadSize);
        }

        Packet(const std::size_t payloadSize, std::span<const std::byte> data)
        {
            m_payload.resize(payloadSize);
            unmarshal(data);
        }

        explicit Packet(const std::size_t payloadSize, const std::size_t sequenceIndex)
            : m_timestamp(std::chrono::system_clock::now())
            , m_sequenceIndex(sequenceIndex)
        {
            fillRandomData(m_payload, payloadSize);
            m_checksum = crc8::getInstance().calculate(
                m_timeAsBytes(), std::as_bytes(std::span<const std::size_t>(&m_sequenceIndex, 1)), m_payload);
        }

        auto marshal()
        {
            std::vector<std::byte> out;

            uint64_t timestamp = m_timestamp.time_since_epoch().count();

            appendData(out, timestamp);
            appendData(out, m_sequenceIndex);
            out.insert(out.end(), m_payload.begin(), m_payload.end());
            appendData(out, m_checksum);

            return out;
        }

        std::size_t unmarshal(const std::span<const std::byte> data)
        {
            std::size_t iterator = 0;

            // Extract time
            auto timeCount = extractData<uint64_t>(data, iterator);
            if (!timeCount)
            {
                return iterator;
            }

            m_timestamp = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(timeCount.value()));

            // Extract sequence number
            auto sequenceIndex = extractData<std::size_t>(data, iterator);
            if (!sequenceIndex)
            {
                return iterator;
            }

            m_sequenceIndex = sequenceIndex.value();

            // Extract payload
            if (iterator >= data.size() || data.size() < m_payload.size())
            {
                return iterator;
            }

            std::print("Payload Size: {:d}\n", m_payload.size());

            m_payload = std::ranges::to<std::vector<std::byte>>(data.subspan(iterator, m_payload.size()));
            iterator += m_payload.size();

            // Extract checksum
            auto checksum = extractData<std::byte>(data, iterator);
            if (!checksum)
            {
                return iterator;
            }

            m_checksum = checksum.value();

            return iterator;
        }

        bool valid() const
        {
            return m_checksum ==
                   crc8::getInstance().calculate(
                       m_timeAsBytes(), std::as_bytes(std::span<const std::size_t>(&m_sequenceIndex, 1)), m_payload);
        }

        const auto& getTime() const noexcept
        {
            return m_timestamp;
        }

        const auto& getSequenceNumber() const noexcept
        {
            return m_timestamp;
        }

        const auto& getPayload() const noexcept
        {
            return m_timestamp;
        }

        const auto& getChecksum() const noexcept
        {
            return m_timestamp;
        }

        bool operator==(const Packet& other) const
        {
            if (m_timestamp != other.m_timestamp)
            {
                return false;
            }

            if (m_sequenceIndex != other.m_sequenceIndex)
            {
                return false;
            }

            if (m_payload != other.m_payload)
            {
                return false;
            }

            return m_checksum == other.m_checksum;
        }

        void debug() const
        {
            std::print("Timestamp: {:s}\n", std::format("{:%Y-%m-%d %H:%M:%S}", m_timestamp));
            std::print("Sequence index: {:d}\n", m_sequenceIndex);
            std::print("Payload [{:d}] body: ", m_payload.size());
            std::for_each(
                m_payload.begin(),
                m_payload.end(),
                [&](const auto& item) { std::print("{:02X} ", static_cast<uint8_t>(item)); });
            std::print("\nChecksum: 0x{:X}\n", static_cast<uint8_t>(m_checksum));
        }

    private:
        const std::span<const std::byte> m_timeAsBytes() const
        {
            const uint64_t timestamp = m_timestamp.time_since_epoch().count();
            return std::as_bytes(std::span<const uint64_t>(&timestamp, 1));
        }

        template <typename T, typename U>
        static void appendData(U& output, const T& input)
        {
            auto temp = std::as_bytes(std::span<const T>(&input, 1));
            output.insert(output.end(), temp.begin(), temp.end());
        }

        template <typename T, typename U>
        static std::optional<T> extractData(const U& input, std::size_t& iterator)
        {
            if (iterator >= input.size() || input.size() < sizeof(uint64_t))
            {
                return {};
            }

            auto dataBytes = input.subspan(iterator, sizeof(T));
            auto value = *reinterpret_cast<T*>(const_cast<std::byte*>(dataBytes.data()));
            iterator += sizeof(T);

            return value;
        }

    private:
        std::chrono::system_clock::time_point m_timestamp;
        std::size_t m_sequenceIndex = 0;
        std::vector<std::byte> m_payload = {};
        std::byte m_checksum = std::byte(0x00);
    };
}
