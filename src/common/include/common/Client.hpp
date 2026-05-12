#pragma once

#include "common/Pipe.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <string_view>
#include <print>

namespace common
{
    class Client : public Pipe<Client>
    {
    public:
        constexpr static std::string_view kLogPrefix = "[Client] ";

        int start()
        {
            std::print("{:s} Starting\n", kLogPrefix);

            auto result =
                connect(this->m_socket, reinterpret_cast<struct sockaddr*>(&this->m_addr), sizeof(this->m_addr));
            if (-1 == result)
            {
                std::print("{:s} Error connecting: 0x{:X}\n", kLogPrefix, static_cast<uint32_t>(result));
            }

            return result;
        }

        int stop()
        {
            return 0;
        }

        const auto& getSocket() const noexcept
        {
            return m_socket;
        }
    };
}
