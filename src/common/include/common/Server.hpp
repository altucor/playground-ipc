#pragma once

#include "common/Pipe.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <print>
#include <thread>
#include <atomic>

namespace common
{
    class Server : public Pipe<Server>
    {
    public:
        constexpr static std::string_view kLogPrefix = "[Server] ";

        inline bool clientConnected() const
        {
            return m_clientConnected.load();
        }

        int start()
        {
            std::print("{:s} Starting\n", kLogPrefix);

            unlink(kPipeName.data());
            m_clientConnected.store(false);

            int result = bind(this->m_socket, reinterpret_cast<struct sockaddr*>(&m_addr), sizeof(m_addr));
            if (result)
            {
                std::print("{:s} Error binding: 0x{:X}\n", kLogPrefix, static_cast<uint32_t>(result));
                return result;
            }

            result = listen(this->m_socket, 5);
            if (result)
            {
                std::print("{:s} Error listener start: 0x{:X}\n", kLogPrefix, static_cast<uint32_t>(result));
                return result;
            }

            m_workerThread = std::jthread(
                [&](std::stop_token token)
                {
                    std::print("{:s} Started accept listener\n", kLogPrefix);

                    while (!token.stop_requested())
                    {
                        if (-1 != m_clientSocket)
                        {
                            continue;
                        }

                        m_clientSocket = accept(this->m_socket, NULL, NULL);
                        if (-1 == m_clientSocket)
                        {
                            std::print("{:s} Error accepting client\n", kLogPrefix);
                            continue;
                        }

                        std::print(
                            "{:s} Accepted new client: 0x{:X}\n", kLogPrefix, static_cast<uint32_t>(m_clientSocket));
                        m_clientConnected.store(true);
                        break;
                    }

                    std::print("{:s} Stopped accept listener\n", kLogPrefix);
                });

            return 0;
        }

        int stop()
        {
            m_workerThread.request_stop();

            close(m_clientSocket);
            m_clientSocket = -1;

            unlink(kPipeName.data());

            return 0;
        }

        const auto& getSocket() const noexcept
        {
            return m_clientSocket;
        }

    private:
        int m_clientSocket = -1;
        std::jthread m_workerThread;
        std::atomic_bool m_clientConnected;
    };
}
