#pragma once

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <print>
#include <thread>
#include <atomic>

namespace keyboard
{
    class EventHandler
    {
    public:
        EventHandler(EventHandler& other) = delete;
        void operator=(EventHandler const& other) = delete;

        ~EventHandler()
        {

            m_stopSource.request_stop();

            if (m_worker.joinable())
            {
                m_worker.join();
            }
        }

        static EventHandler& getInstance()
        {
            static EventHandler instance;
            return instance;
        }

        bool shouldPause() const noexcept
        {
            return m_pauseFlag.load();
        }

    private:
        EventHandler()
        {

            m_worker = std::thread(
                [&]()
                {
                    while (!m_stopSource.stop_requested())
                    {
                        const auto keyCode = getKeyNonBlocking(m_stopSource);
                        if (keyCode == 0)
                        {
                            std::print("[Keyboard] Got invalid key symbol, error occured, stopping\n");
                            break;
                        }

                        constexpr uint8_t kPauseKey = 112; // On MacOS letter "P"
                        if (keyCode == kPauseKey)
                        {
                            m_pauseFlag.store(!m_pauseFlag.load());
                            std::print(
                                "[Keyboard] Got keypress of ({:s}[{:d}]), pause flag set to {:d}\n",
                                "P",
                                kPauseKey,
                                m_pauseFlag.load());
                        }
                    }
                });
        }

        static char getKeyNonBlocking(std::stop_source stopSource)
        {
            struct termios term = {0};

            if (tcgetattr(0, &term) < 0)
            {
                std::print("[Keyboard] Failed tcgetattr TCSANOW");
                return 0;
            }

            term.c_lflag &= ~ICANON;
            term.c_lflag &= ~ECHO;
            term.c_cc[VMIN] = 1;
            term.c_cc[VTIME] = 0;

            if (tcsetattr(0, TCSANOW, &term) < 0)
            {
                std::print("[Keyboard] Failed tcsetattr TCSANOW");
                return 0;
            }

            std::size_t availableCount = 0;
            while (!stopSource.stop_requested())
            {
                ioctl(0, FIONREAD, &availableCount);
                if (availableCount)
                {
                    break;
                }
            }

            if (stopSource.stop_requested())
            {
                std::print("[Keyboard] Stop requested, exiting\n");
                return 0;
            }

            char buf = 0;
            if (read(0, &buf, 1) < 0)
            {
                std::print("[Keyboard] Failed read");
                return 0;
            }

            term.c_lflag |= ICANON;
            term.c_lflag |= ECHO;
            if (tcsetattr(0, TCSADRAIN, &term) < 0)
            {
                std::print("[Keyboard] Failed tcsetattr TCSADRAIN");
                return 0;
            }

            return buf;
        }

    private:
        std::stop_source m_stopSource;
        std::thread m_worker;
        std::atomic<bool> m_pauseFlag;
    };
};
