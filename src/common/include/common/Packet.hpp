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
        Packet()
        {
        }

        explicit Packet(const std::size_t payloadSize)
        {
            m_payload.resize(payloadSize);
        }

        Packet(const std::size_t payloadSize, std::span<const std::byte> data)
        {
            m_payload.resize(payloadSize);
            unmarshal(data);
        }

        explicit Packet(const std::size_t payloadSize, const std::size_t sequenceNumber)
            : m_timestamp(std::chrono::system_clock::now())
            , m_sequenceNumber(sequenceNumber)
        {
            fillRandomData(m_payload, payloadSize);
            m_checksum = m_calculateChecksum();
        }

        auto marshal()
        {
            std::vector<std::byte> out;

            uint64_t timestamp = m_timestamp.time_since_epoch().count();

            appendData(out, timestamp);
            appendData(out, m_sequenceNumber);
            appendData(out, static_cast<uint64_t>(m_payload.size()));
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
            auto sequenceNumber = extractData<std::size_t>(data, iterator);
            if (!sequenceNumber)
            {
                return iterator;
            }

            m_sequenceNumber = sequenceNumber.value();

            // Extract payload size
            auto payloadSize = extractData<std::size_t>(data, iterator);
            if (payloadSize)
            {
                // Extract payload
                m_payload.resize(payloadSize.value());

                if (iterator >= data.size() || data.size() < m_payload.size())
                {
                    return iterator;
                }

                std::print("Payload Size: {:d}\n", m_payload.size());

                m_payload = std::ranges::to<std::vector<std::byte>>(data.subspan(iterator, m_payload.size()));
                iterator += m_payload.size();
            }

            // Extract checksum
            auto checksum = extractData<std::byte>(data, iterator);
            if (!checksum)
            {
                return iterator;
            }

            m_checksum = checksum.value();

            return iterator;
        }

        template <typename T>
        std::size_t unmarshal(T& pipe)
        {
            std::size_t totalRead = 0;
            std::size_t lastRead = 0;

            uint64_t timeCount = 0;
            lastRead = pipe.tryReadValue(timeCount);
            if (lastRead != sizeof(timeCount))
            {
                std::print("Failed to read timestamp value\n");
                return totalRead;
            }
            m_timestamp = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(timeCount));
            totalRead += lastRead;

            lastRead = pipe.tryReadValue(m_sequenceNumber);
            if (lastRead != sizeof(m_sequenceNumber))
            {
                std::print("Failed to read sequence index value\n");
                return totalRead;
            }
            totalRead += lastRead;

            std::size_t payloadSize = 0;
            lastRead = pipe.tryReadValue(payloadSize);
            if (lastRead != sizeof(payloadSize))
            {
                std::print("Failed to read sequence index value\n");
                return totalRead;
            }
            totalRead += lastRead;

            std::print("[Packet] Unmarshal from pipe payload size: 0x{:02X}\n", payloadSize);

            if (payloadSize)
            {
                m_payload.resize(payloadSize);
                auto stopToken = std::stop_token();
                lastRead = pipe.tryReadSome(m_payload, stopToken);
                if (lastRead != payloadSize)
                {
                    std::print("Failed to read payload of size: {:d}\n", payloadSize);
                    return totalRead;
                }
                totalRead += lastRead;
            }

            lastRead = pipe.tryReadValue(m_checksum);
            if (lastRead != sizeof(m_checksum))
            {
                std::print("Failed to read sequence index value\n");
                return totalRead;
            }
            totalRead += lastRead;

            return totalRead;
        }

        bool valid() const
        {
            auto temp = m_calculateChecksum();
            std::print(
                "Checksum current: 0x{:02X} - calculated: 0x{:02X}\n",
                static_cast<uint8_t>(m_checksum),
                static_cast<uint8_t>(temp));
            return m_checksum == temp;
        }

        const auto& getTime() const noexcept
        {
            return m_timestamp;
        }

        const auto& getSequenceNumber() const noexcept
        {
            return m_sequenceNumber;
        }

        const auto& getPayload() const noexcept
        {
            return m_payload;
        }

        const auto& getChecksum() const noexcept
        {
            return m_checksum;
        }

        bool operator==(const Packet& other) const
        {
            if (m_timestamp != other.m_timestamp)
            {
                return false;
            }

            if (m_sequenceNumber != other.m_sequenceNumber)
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
            std::print("Sequence number: {:d}\n", m_sequenceNumber);
            std::print("Payload [{:d}] body: ", m_payload.size());

            constexpr std::size_t kPayloadDebugSizePrefix = 4;
            std::for_each(
                m_payload.begin(),
                std::next(m_payload.begin(), kPayloadDebugSizePrefix),
                [&](const auto& item) { std::print("{:02X} ", static_cast<uint8_t>(item)); });

            std::print("... ");

            constexpr std::size_t kPayloadDebugSizeSuffix = 4;
            std::for_each(
                std::next(m_payload.begin(), m_payload.size() - kPayloadDebugSizeSuffix),
                m_payload.end(),
                [&](const auto& item) { std::print("{:02X} ", static_cast<uint8_t>(item)); });

            std::print("\nChecksum: 0x{:X}\n", static_cast<uint8_t>(m_checksum));
        }

    private:
        const auto m_timeAsBytes() const
        {
            const uint64_t timestamp = m_timestamp.time_since_epoch().count();
            std::array<std::byte, sizeof(timestamp)> output;
            std::memcpy(output.data(), &timestamp, sizeof(timestamp));
            return output;
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

        const std::byte m_calculateChecksum() const
        {
            return crc8::getInstance().calculate(
                m_timeAsBytes(), std::as_bytes(std::span<const std::size_t>(&m_sequenceNumber, 1)), m_payload);
        }

    private:
        std::chrono::system_clock::time_point m_timestamp;
        std::size_t m_sequenceNumber = 0;
        std::vector<std::byte> m_payload = {};
        std::byte m_checksum = std::byte(0x00);
    };
}
