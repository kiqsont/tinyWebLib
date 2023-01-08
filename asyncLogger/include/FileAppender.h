#ifndef ASYNC_LOGGER_FILEAPPENDER
#define ASYNC_LOGGER_FILEAPPENDER

#include "noncopyable.h"
#include <cstdio>
#include <sys/types.h>
#include <string>

namespace asyncLogger::detail
{
    class FileAppender : noncopyable
    {
    public:
        explicit FileAppender(const std::string &filename);
        ~FileAppender() noexcept;
        void append(const char *message, size_t len);
        void flush();
        off64_t writtenBytes() const { return m_writenBytes; }
        void resetWritten() { m_writenBytes = 0; }

    private:
        size_t write(const char *message, size_t len);
        void init(const std::string &filename);

    private:
        FILE *m_file = nullptr;
        // char m_buffer[64 * 1024]{0}; // 64k buffer
        off64_t m_writenBytes = 0;
    };
}

#endif // ASYNC_LOGGER_FILEAPPENDER