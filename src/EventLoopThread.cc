#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : thread_(std::bind(&EventLoopThread::threadFunc, this), name), callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start();

    // EventLoop *loop = nullptr;  // ?
    {
        std::unique_lock<std::mutex> ul(mutex_);
        cond_.wait(ul, [this]()
                   { return loop_ != nullptr; });
        // loop = loop_;
    }
    return loop_;
}

// run in new thread
void EventLoopThread::threadFunc()
{
    EventLoop loop; // one loop per thread

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> ul(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> ul(mutex_);
    loop_ = nullptr;
}