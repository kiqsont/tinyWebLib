#ifndef ASYNC_LOGGER_FIXEDBUFFER
#define ASYNC_LOGGER_FIXEDBUFFER

#include <cstring>
#include <string>
#include "noncopyableLog.h"

namespace asyncLogger::asyncLoggerDetail
{
    constexpr int kSmallBuffer = 4096;        // 4k
    constexpr int kLargeBuffer = 4069 * 1024; // 4m

    template <int SIZE>
    class FixedBuffer : noncopyableLog
    {
    public:
        constexpr FixedBuffer() : m_cur_buf(m_buffer) {}
        int avail() { return static_cast<int>(end() - m_cur_buf); }
        int size() { return static_cast<int>(m_cur_buf - m_buffer); }
        char *data() { return m_buffer; }
        void append(const char *message, size_t len)
        {
            if (avail() > len)
            {
                memcpy(m_cur_buf, message, len);
                m_cur_buf += len;
            }
        }
        void bzero() { memset(m_buffer, 0, sizeof m_buffer); }
        void reset()
        {
            m_cur_buf = m_buffer;
            m_buffer[1] = 0;
        }

    private:
        char *end()
        {
            return m_buffer + sizeof m_buffer;
        }

    private:
        char m_buffer[SIZE]{0};    // buffer
        char *m_cur_buf = nullptr; // pointer to buffer
    };
}

#endif // ASYNC_LOGGER_FIXEDBUFFER