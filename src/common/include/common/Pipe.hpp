#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include <memory>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>
#include <span>
#include <print>
#include <thread>

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
            if (!isSocketValid())
            {
                return 0;
            }

            return ::read(static_cast<DerivedT*>(this)->getSocket(), reinterpret_cast<void*>(data.data()), data.size());
        }

        std::size_t write(const std::span<const std::byte> data)
        {
            if (!isSocketValid())
            {
                return 0;
            }

            return ::write(static_cast<DerivedT*>(this)->getSocket(), data.data(), data.size());
        }

    private:
        bool isSocketValid()
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
    };
}
