#pragma once

#include "common/Utils.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>

#include <memory>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>
#include <span>
#include <print>
#include <stop_token>

namespace common
{
    constexpr static std::string_view kPipeName = "/tmp/Playground-ipc";

    template <typename DerivedT>
    class Pipe
    {
    public:
        Pipe()
        {
            // TODO: Maybe better to use SOCK_DGRAM to use it as unreliable transport
            // We can assume here that our needs and protocol allows dropping data
            m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
            if (-1 == m_socket)
            {
                std::print("{:s} Error creating socket\n", static_cast<DerivedT*>(this)->kLogPrefix);
                return;
            }

            std::print(
                "{:s} Opened pipe socket: 0x{:X}\n",
                static_cast<DerivedT*>(this)->kLogPrefix,
                static_cast<uint32_t>(m_socket));

            std::memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sun_family = AF_UNIX;

            static_assert(
                kPipeName.size() < sizeof(m_addr.sun_path), "Pipe name longer then available socket address buffer");

            std::copy_n(kPipeName.data(), kPipeName.size(), reinterpret_cast<char*>(&m_addr.sun_path));
        }

        ~Pipe()
        {
            close(m_socket);
            m_socket = -1;
        }

        auto start()
        {
            return static_cast<DerivedT*>(this)->start();
        }

        auto stop()
        {
            return static_cast<DerivedT*>(this)->stop();
        }

        std::size_t read(std::span<std::byte> data)
        {
            if (!m_isSocketValid())
            {
                return 0;
            }

            const std::size_t totalRead =
                ::read(static_cast<DerivedT*>(this)->getSocket(), reinterpret_cast<void*>(data.data()), data.size());

            m_recvPerfCounter.updateBatch(totalRead);

            return totalRead;
        }

        const std::size_t tryReadSome(
            std::span<std::byte> data,
            const std::chrono::milliseconds& timeout = std::chrono::milliseconds(1000))
        {
            struct pollfd pollFd[1] = {};
            pollFd[0].fd = static_cast<DerivedT*>(this)->getSocket();
#if defined(__linux)
            pollFd[0].events = POLLIN;
#elif defined(__APPLE__)
            pollFd[0].events = POLL_IN;
#endif

            constexpr auto timeoutOneShot = std::chrono::milliseconds(10);

            auto startTime = std::chrono::steady_clock::now();
            while (!static_cast<DerivedT*>(this)->getStopSource().stop_requested() &&
                   std::chrono::steady_clock::now() - startTime < timeout)
            {
                if (poll(pollFd, 1, timeoutOneShot.count()) != 1)
                {
                    continue;
                }

                std::size_t availableCount = 0;
                ioctl(static_cast<DerivedT*>(this)->getSocket(), FIONREAD, &availableCount);

                // std::print("IOCTL available for read: 0x{:08X} - {:d}\n", availableCount, availableCount);

                if (data.size() <= availableCount)
                {
                    return read(data);
                }
            }

            return 0;
        }

        template <typename T>
        const std::size_t tryReadValue(T& output)
        {
            auto outputBuffer = std::as_writable_bytes(std::span<T>(&output, 1));
            return tryReadSome(outputBuffer);
        }

        std::size_t write(const std::span<const std::byte> data)
        {
            if (!m_isSocketValid())
            {
                return 0;
            }

            constexpr std::size_t kChunkSize = 64;
            const std::size_t remainingBytes = data.size() % kChunkSize;
            const std::size_t iterationsTotal = data.size() / kChunkSize + (remainingBytes == 0 ? 0 : 1);

            std::size_t written = 0;

            for (std::size_t i = 0; i < iterationsTotal; i++)
            {
                if (static_cast<DerivedT*>(this)->getStopSource().stop_requested())
                {
                    return written;
                }

                // Handle remainder bytes
                std::size_t amountToWrite = kChunkSize;
                if (i == iterationsTotal - 1 && remainingBytes != 0)
                {
                    amountToWrite = remainingBytes;
                }

                written += ::write(
                    static_cast<DerivedT*>(this)->getSocket(),
                    const_cast<std::byte*>(&*std::next(data.begin(), i * kChunkSize)),
                    amountToWrite);

                m_sendPerfCounter.updateBatch(written);
            }

            return written;
        }

        auto& getSendPerfCounter()
        {
            return m_sendPerfCounter;
        }

        auto& getRecvPerfCounter()
        {
            return m_recvPerfCounter;
        }

    private:
        bool m_isSocketValid()
        {
            if (-1 == static_cast<DerivedT*>(this)->getSocket())
            {
                return false;
            }

            if (fcntl(static_cast<DerivedT*>(this)->getSocket(), F_GETFD) == -1)
            {
                return false;
            }

            return true;
        }

    protected:
        int m_socket = -1;
        struct sockaddr_un m_addr = {};
        common::PerformanceCounter m_sendPerfCounter;
        common::PerformanceCounter m_recvPerfCounter;
    };
}
