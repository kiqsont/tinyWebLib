#ifndef KIQSONT_MUDUO_COPY_TIMERQUEUE
#define KIQSONT_MUDUO_COPY_TIMERQUEUE

#include "Callbacks.h"
#include "Timestamp.h"
#include "noncopyable.h"
#include "Channel.h"
#include "TimerID.h"

#include <atomic>
#include <set>
#include <vector>
#include <memory>

/**
 *  use std::set<std::unique_ptr<Timer>> to sort the timers' sequence
 *  set sorted by expriation(Timestamp) and sequece(int64_t) of Timer
 *  when the first timer timeout, the timerfd is readable can EventLoop's poller can know it
 */

class EventLoop;

class Timer : public TimerID, public noncopyable
{
public:
    friend class TimerQueue;

public:
    Timer(TimerCallback cb, Timestamp when, double interval = 0)
        : TimerID(when), timerCallback_(std::move(cb)), interval_(interval), repeat_(interval > 0)
    {
    }

    Timer(Timestamp when, uint64_t num, double interval = 0) : TimerID(when, num), interval_(interval), repeat_(interval > 0) {}

    void run() const
    {
        timerCallback_();
    }

    bool repeat() const
    {
        return repeat_;
    }

    void restart(Timestamp now);

private:
    TimerCallback timerCallback_;
    const double interval_;
    const bool repeat_;
};

struct CompableTimer
{
    bool operator()(const std::unique_ptr<Timer> &t1, const std::unique_ptr<Timer> &t2) const
    {
        if (t1->expiration_ == t2->expiration_)
            return t1->sequence_ < t2->sequence_;
        return t1->expiration_ < t2->expiration_;
    }
};

class TimerQueue : noncopyable
{
public:
    using TimerList = std::set<std::unique_ptr<Timer>, CompableTimer>;

public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerID addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerID timerID);
    void cancelAll();

private:
    void addTimerInLoop(Timer *timer);
    void cancelInLoop(TimerID timerID);
    void cancelAllInLoop();

    void handleRead();
    std::vector<std::unique_ptr<Timer>> getExpired(Timestamp now);
    void reset(std::vector<std::unique_ptr<Timer>> &expired, Timestamp now);
    bool insert(std::unique_ptr<Timer> timer);

private:
    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
    std::set<int64_t> cancelingTimers_;
    std::atomic_bool cancelingExpiredTimers_{false};
    std::atomic_bool cancelAllTimers_{false};
};
#endif // KIQSONT_MUDUO_COPY_TIMERQUEUE