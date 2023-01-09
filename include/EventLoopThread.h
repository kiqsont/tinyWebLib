#ifndef KIQSONT_MUDUO_OVERWRITE_EVENTLOOPTHREAD
#define KIQSONT_MUDUO_OVERWRITE_EVENTLOOPTHREAD

#include "noncopyable.h"
#include "Thread.h"

#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

public:
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());

    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

private:
    EventLoop *loop_ = nullptr;
    bool exiting_ = false;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

#endif // KIQSONT_MUDUO_OVERWRITE_EVENTLOOPTHREAD