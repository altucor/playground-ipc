#pragma once

#include <cstdint>
#include <span>
#include <chrono>
#include <atomic>
#include <limits>

namespace common
{
    template <std::size_t N>
    static const auto makeBytes(const uint8_t (&arr)[N])
    {
        return std::as_bytes(std::span<const uint8_t>(arr));
    }

    static inline bool isNewer(const uint64_t current, const uint64_t previous)
    {
        // The magic is in casting the result of the unsigned subtraction to a signed integer.
        // This correctly handles the wraparound case.
        // If current is slightly larger than previous, the result is a small positive number.
        // If current has wrapped around and previous is large, e.g., current=5, previous=UINT32_MAX,
        // the unsigned subtraction (5 - 4294967295) results in 6.
        // Casting 6 to int32_t is still 6, so 6 > 0, and the function correctly returns true.
        return static_cast<int64_t>(current - previous) > 0;
    }

    class PerformanceCounter
    {
    public:
        PerformanceCounter(const std::chrono::milliseconds& updateInterval = std::chrono::milliseconds(1000))
            : m_updateInterval(updateInterval)
        {
        }

        void updateInterval(const std::chrono::milliseconds& updateInterval)
        {
            m_updateInterval = updateInterval;
        }

        const auto get() const noexcept
        {
            return m_count.load();
        }

        const auto getAbsoluteMaximum() const noexcept
        {
            return m_absoluteMaximum;
        }

        const auto getAbsoluteMinimum() const noexcept
        {
            return m_absoluteMinimum;
        }

        void updateBatch(const std::size_t amount)
        {
            m_updateCounter += amount;
            if (std::chrono::steady_clock::now() - m_lastUpdateTimestamp > m_updateInterval)
            {
                m_count.store(m_updateCounter * (std::chrono::milliseconds(1000).count() / m_updateInterval.count()));
                if (m_absoluteMaximum < m_count.load())
                {
                    m_absoluteMaximum = m_count.load();
                }
                if (m_count.load() < m_absoluteMinimum)
                {
                    m_absoluteMinimum = m_count.load();
                }
                m_updateCounter = 0;
                m_lastUpdateTimestamp = std::chrono::steady_clock::now();
            }
        }

        void update()
        {
            updateBatch(1);
        }

    private:
        std::chrono::milliseconds m_updateInterval = std::chrono::milliseconds(1000);
        std::chrono::steady_clock::time_point m_lastUpdateTimestamp = std::chrono::steady_clock::now();
        uint64_t m_updateCounter = 0;
        std::atomic<std::size_t> m_count{0}; // updates per second
        uint64_t m_absoluteMaximum = 0;
        uint64_t m_absoluteMinimum = std::numeric_limits<uint64_t>::max();
    };
}
