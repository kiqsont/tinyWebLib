#ifndef ASYNCE_LOGGER_CONFIG
#define ASYNCE_LOGGER_CONFIG

#include "fmt/format.h"
#include <cstdio>
#include <functional>

namespace asyncLogger
{
    enum PRINT_FLAG
    {
        Ldata = 1 << 0,
        Ltime = 1 << 1,
        Llongname = 1 << 2,
        Lshortname = 1 << 3,
        Lline = 1 << 4,
        LthreadId = 1 << 5,
        LstdFlags = Ldata | Ltime | Lshortname | Lline
    };

    enum LEVEL
    {
        L_DEBUG,
        L_INFO,
        L_WARN,
        L_ERROR,
        L_FATAL,
        LEVEL_COUNT
    };

    namespace asyncLoggerDetail
    {
        using callback_t = std::function<void(fmt::memory_buffer &)>;

        class Config
        {
        public:
            static Config &instance();

            const char *&basename()
            {
                return m_log_basename;
            }
            int &flags()
            {
                return m_log_flags;
            }
            bool &console()
            {
                return m_log_console;
            }
            int &rollSize()
            {
                return m_log_rollSize;
            }
            int &flushInterval()
            {
                return m_log_flushInterval;
            }
            const char *&prefix()
            {
                return m_log_prefix;
            }
            callback_t &before()
            {
                return m_before;
            }
            callback_t &end()
            {
                return m_end;
            }

        private:
            int m_log_flags = LstdFlags;
            bool m_log_console = true;
            int m_log_rollSize = 20 * 1024 * 1024; // defalut log file size is 20M
            int m_log_flushInterval = 3;           // flush every 3s
            const char *m_log_basename = nullptr;
            const char *m_log_prefix = nullptr;
            callback_t m_before;
            callback_t m_end;
        };
    }
}

#endif // ASYNCE_LOGGER_CONFIG