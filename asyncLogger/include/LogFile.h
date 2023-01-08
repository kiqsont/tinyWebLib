#ifndef ASYNC_LOGGER_LOGFILE
#define ASYNC_LOGGER_LOGFILE

#include "FileAppender.h"
#include "noncopyable.h"
#include <ctime>
#include <sys/types.h>
#include <memory>
#include <mutex>

namespace asyncLogger::detail
{
    class LogFile : noncopyable
    {
    public:
        explicit LogFile(const std::string &basename, off64_t rollSize, bool threadSafe = true, int flushInterval = 3, int checkEveryN = 1024);

        void append(const char *message, size_t len);
        void flush();
        void rollFile(const time_t *now = nullptr);

    private:
        void append_unlocked(const char *message, size_t len);

    private:
        const int kRollPerSeconds = 60 * 60 * 24;
        const off64_t m_rollSize; // set in Config.h default for 20M
        const int m_flushInterval;
        const int m_checkEveryN; // flush or rollFile in every m_checkEveryN
        int m_count = 0;         // count the number for m_checkEveryN

        const std::string m_basename;
        std::unique_ptr<std::mutex> m_mutex; // make_unique mutex_resource in thread_safe flag
        time_t m_lastPeriod{0};
        time_t m_lastRoll{0};
        time_t m_lastFlush{0};
        std::unique_ptr<FileAppender> m_file = nullptr;
    };
}

#endif // ASYNC_LOGGER_LOGFILE