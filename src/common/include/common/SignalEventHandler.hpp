#pragma once

#include <signal.h>

#include <atomic>

namespace signaling
{
    class EventHandler
    {
    public:
        static EventHandler& getInstance()
        {
            static EventHandler instance;
            return instance;
        }

        static void sigHandler(int sig)
        {
            getInstance().m_universalHandler(sig);
        }

        bool shouldPause() const noexcept
        {
            return m_pauseFlag.load();
        }

    private:
        EventHandler()
        {
            ::signal(SIGSTOP, &EventHandler::sigHandler);
            ::signal(SIGCONT, &EventHandler::sigHandler);
        }

        void m_universalHandler(const int sig)
        {
            switch (sig)
            {
                case SIGSTOP:
                {
                    m_pauseFlag.store(true);
                    break;
                }

                case SIGCONT:
                {
                    m_pauseFlag.store(false);
                    break;
                }

                default: break;
            }
        }

    private:
        std::atomic<bool> m_pauseFlag;
    };
}
