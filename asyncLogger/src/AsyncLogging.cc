#include "AsyncLogging.h"

#include <cassert>
#include <utility>
#include <chrono>
#include "Logger.h"
#include "LogFile.h"
#include "LoggerUtil.h"

using namespace asyncLogger::detail;
using namespace asyncLogger;

AsyncLogging::AsyncLogging(const std::string &basename, off64_t rollSize, int flushInterval)
    : m_basename(basename), m_rollSize(rollSize), m_flushInterval(flushInterval), m_done(false), m_latch(1)
{
    try
    {
        m_curBuffer = std::make_unique<Buffer>();
        m_nextBuffer = std::make_unique<Buffer>();
        trace("background log thread beginning to build");

        m_thread = std::make_unique<std::thread>([this]
                                                 { thread_worker(); });
        trace("begin to init background thread resource");
        m_latch.wait();
        trace("after background thread init");
    }
    catch (...)
    {
        trace("create worker thread failed or buffer init failed");
        do_done();
        throw std::runtime_error("AsyncLogging create thread or buffer init failed");
    }
}

AsyncLogging::~AsyncLogging() noexcept
{
    trace("AsyncLogging dtor");
    do_done();
}

void AsyncLogging::do_done()
{
    m_done.store(true);

    trace("do_done:1.notify the thread");
    m_cv.notify_one();

    if (m_thread && m_thread->joinable())
    {
        trace("do_done:2.join the worker thread");
        m_thread->join();
    }
}

void AsyncLogging::append(const char *message, size_t len)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    if (m_curBuffer->avail() > len) // has enough buffer
    {
        m_curBuffer->append(message, len);
        return;
    }

    trace("AsyncLogging buffer is full,notify the worker thread");
    m_buffers.emplace_back(std::move(m_curBuffer));
    if (m_nextBuffer)
    {
        trace("has spare buffer(nextBuffer)");
        m_curBuffer = std::move(m_nextBuffer);
    }
    else
    {
        trace("hasn't spare buffer, and create new buffer space");
        try
        {
            m_curBuffer = std::make_unique<Buffer>();
        }
        catch (const std::exception &e)
        {
            do_done();
            throw std::runtime_error(e.what());
        }
    }
    m_curBuffer->append(message, len);
    m_cv.notify_one();
}

void AsyncLogging::thread_worker()
{
    try
    {
        LogFile output(m_basename, m_rollSize, false); // Not thread safe
        BufferPtr newBuffer1 = std::make_unique<Buffer>();
        BufferPtr newBuffer2 = std::make_unique<Buffer>();
        BufferVectorPtr buffersToWrite; // to reduce critical zone
        buffersToWrite.reserve(16);
        static bool once = false;

        while (true)
        {
            if (m_done)
                break;

            trace("new loop for async logging");
            {
                std::unique_lock<std::mutex> ul(m_mutex);
                trace("get unique_lock");
                if (m_buffers.empty())
                {
                    if (!once)
                    {
                        trace("first time to run worker thread");
                        once = true;
                        m_latch.countDown();
                    }
                    m_cv.wait_for(ul, std::chrono::seconds(m_flushInterval));
                    trace("worker thread wake up or timeout {:d}s", m_flushInterval);
                }

                m_buffers.emplace_back(std::move(m_curBuffer));
                m_curBuffer = std::move(newBuffer1);
                buffersToWrite.swap(m_buffers);

                if (!m_nextBuffer)
                {
                    trace("update nextBuffer");
                    m_nextBuffer = std::move(newBuffer2);
                }
            }
            // end for unique_lock(critical zone)

            // hasn't log data to write
            if (buffersToWrite.empty())
                continue;

            if (buffersToWrite.size() > 25) // too much log data, drop some buffer data
            {
                trace("too much log data, drop log data but save 2 buffer");
                char buf[256]{0};
                snprintf(buf, sizeof buf, "Dropped log data at %s, %zd larger buffers", Util::getCurDateTime(true), buffersToWrite.size());
                fputs(buf, stderr);
                output.append(buf, static_cast<size_t>(strlen(buf)));
                buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
            }

            // begin to write
            trace("Begin to write log data");
            for (const auto &buffer : buffersToWrite)
            {
                output.append(buffer->data(), buffer->size());
            }

            // clean needless buffer
            if (buffersToWrite.size() > 2)
            {
                buffersToWrite.resize(2);
            }

            // reset newBuffer1
            if (!newBuffer1)
            {
                trace("newBuffer1 update");
                assert(!buffersToWrite.empty());
                newBuffer1 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
                newBuffer1->reset();
            }
            // reset newBuffer2
            if (!newBuffer2)
            {
                trace("newBuffer2 update");
                assert(!buffersToWrite.empty());
                newBuffer2 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
                newBuffer2->reset();
            }

            buffersToWrite.clear();
            output.flush();
            trace("flush log data");
        }
        output.flush();
        trace("thread normal exit");
    }
    catch (const std::exception &e)
    {
        trace("thread exit error : {}", e.what());
        fprintf(stderr, "thread abnormal exit:%s", e.what());
        m_thread.reset();
    }
}
