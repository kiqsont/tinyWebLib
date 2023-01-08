#include "CountDownLatch.h"

using namespace asyncLogger::asyncLoggerDetail;
CountDownLatch::CountDownLatch(int count)
    : m_count(count)
{
}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> ul(m_mutex);
    m_cv.wait(ul, [this]()
              { return !m_count > 0; });
}

void CountDownLatch::countDown()
{
    std::unique_lock<std::mutex> ul(m_mutex);
    --m_count;
    if (m_count == 0)
    {
        m_cv.notify_all();
    }
}

int CountDownLatch::getCount()
{
    std::unique_lock<std::mutex> ul(m_mutex);
    return m_count;
}