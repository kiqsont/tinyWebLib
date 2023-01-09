#ifndef KIQSONT_MUDUO_COPY_EVENTLOOP
#define KIQSONT_MUDUO_COPY_EVENTLOOP

// has Channel and Poller

#include "noncopyable.h"
#include "Timestamp.h"
#include "TimerID.h"
#include "Callbacks.h"

#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>

#include <iostream>

class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(Functor cb);   // run cb in current thread
    void queueInLoop(Functor cb); // put cb into queue, wake up loop's thread and run cb

    TimerID runAt(Timestamp time, TimerCallback cb);
    TimerID runAfter(double delay, TimerCallback cb);
    TimerID runEvery(double interval, TimerCallback cb);
    void cancel(TimerID timerID);
    void cancelAll();

    void wakeup(); // wake up the thread for this loop

    // for poller
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread()
    {
        return threadId_ == std::this_thread::get_id();
    }

    unsigned long get_thread_id();

private:
    void handleRead();        // wake up
    void doPendingFunctors(); // callback

private:
    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;
    const std::thread::id threadId_;
    Timestamp pollReturnTime_; // the time that poller return channels
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; // mainLoop distribute a subLoop to handle channel when mainLoop gets a new user channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    std::unique_ptr<TimerQueue> timerQueue_;

    std::atomic_bool callingPendingFunctors_; // mark whether loop needs callback
    std::vector<Functor> pendingFunctors_;    // loop's callback function
    std::mutex mutex_;                        // to the vector above;
};

#endif // KIQSONT_MUDUO_COPY_EVENTLOOP
