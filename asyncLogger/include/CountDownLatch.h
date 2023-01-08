#ifndef ASYNC_LOGGER_COUNTDOWNLATCH
#define ASYNC_LOGGER_COUNTDOWNLATCH

#include <condition_variable>
#include <mutex>

namespace asyncLogger::asyncLoggerDetail
{
    class CountDownLatch
    {
    public:
        explicit CountDownLatch(int count);

        void wait();
        void countDown();
        int getCount();

    private:
        std::condition_variable m_cv;
        std::mutex m_mutex;
        int m_count;
    };
}
#endif // ASYNC_LOGGER_COUNTDOWNLATCH