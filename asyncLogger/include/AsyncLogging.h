#ifndef ASYNC_LOGGER_ASYNCLOGGING
#define ASYNC_LOGGER_ASYNCLOGGING

#include "FixedBuffer.hpp"
#include "CountDownLatch.h"
#include "noncopyable.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <condition_variable>

namespace asyncLogger::detail
{
    class AsyncLogging : noncopyable
    {
    public:
        explicit AsyncLogging(const std::string &basename, off64_t rollSize, int flushInterval = 3);

        ~AsyncLogging() noexcept;

        void append(const char *message, size_t len);

    private:
        void do_done();

        void thread_worker();

    private:
        using Buffer = FixedBuffer<kLargeBuffer>;
        using BufferPtr = std::unique_ptr<Buffer>;
        using BufferVectorPtr = std::vector<BufferPtr>;

        const int m_flushInterval;
        const off64_t m_rollSize;
        std::atomic_bool m_done;
        const std::string m_basename;
        std::unique_ptr<std::thread> m_thread;
        CountDownLatch m_latch;
        std::mutex m_mutex;
        std::condition_variable m_cv;

        BufferPtr m_curBuffer;
        BufferPtr m_nextBuffer;
        BufferVectorPtr m_buffers;
    };
}

#endif // ASYNC_LOGGER_ASYNCLOGGING
