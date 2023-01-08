#ifndef KIQSONT_MUDUO_COPY_EVENTLOOPTHREADPOOL
#define KIQSONT_MUDUO_COPY_EVENTLOOPTHREADPOOL

#include "noncopyable.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <thread>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

public:
    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started() const { return started_; }
    const std::string &name() const { return name_; }

private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_ = false;
    int numThreads_ = 0;
    int next_ = 0;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

#endif // KIQSONT_MUDUO_COPY_EVENTLOOPTHREADPOOL