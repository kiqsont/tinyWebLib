#include "TimerQueue.h"

// #define MUDUO_DEBUG

#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>

#include "Logger.h"
#include "EventLoop.h"

using namespace asyncLogger;

namespace detail // encapsule timerfd functions
{
    int createTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0)
        {
            log_fatal("Failed in TimerQueue::detail::timerfd_create");
        }
        return timerfd;
    }

    struct timespec howMuchTimeFromNow(Timestamp when)
    {
        int64_t microseconds = when.getRawTime() - Timestamp::now().getRawTime();
        if (microseconds < 100)
        {
            microseconds = 100;
        }

        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
        return ts;
    }

    void readTimerfd(int timerfd, Timestamp now)
    {
        uint64_t num;
        ssize_t n = ::read(timerfd, &num, sizeof num);
        if (n != sizeof num)
        {
            log_error("TimerQueue::handleRead() reads");
        }
    }

    void resetTimerfd(int timerfd, Timestamp expiration)
    {
        itimerspec newValue{0};
        itimerspec oldValue{0};
        newValue.it_value = howMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
        if (ret)
        {
            log_error("TimerQueue::detail::readTimerfd timerfd_settime Err");
        }
    }
}

void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = now.addTime(interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timerfd_(detail::createTimerfd()), timerfdChannel_(loop, timerfd_)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
}

TimerID TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer *timerPtr = new Timer(std::move(cb), when, interval);
    TimerID timerID(timerPtr->getExpirationTime(), timerPtr->getSequence());
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timerPtr));
    return timerID;
}
void TimerQueue::cancel(TimerID timerID)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerID));
}

void TimerQueue::cancelAll()
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelAllInLoop, this));
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    assert(loop_->isInLoopThread());
    std::unique_ptr<Timer> timerPtr(timer);
    Timestamp tempExpirationTime = timerPtr->getExpirationTime();
    bool earliestChanged = insert(std::move(timerPtr));

    if (earliestChanged)
    {
        detail::resetTimerfd(timerfd_, tempExpirationTime);
    }
}
void TimerQueue::cancelInLoop(TimerID timerID)
{
    std::unique_ptr<Timer> ptr = std::make_unique<Timer>(timerID.getExpirationTime(), timerID.getSequence());
    auto it = timers_.find(ptr);
    if (it != timers_.end())
    {
        log_trace("TimerQueue::cancelInLoop find node in timers_ and erase");
        timers_.erase(it);
    }
    else if (cancelingExpiredTimers_)
    {
        log_trace("TimerQueue::cancelInLoop add node to cancelingTimers_");
        cancelingTimers_.insert(ptr->getSequence());
    }
}

void TimerQueue::cancelAllInLoop()
{
    timers_.clear();
    cancelAllTimers_.store(true);
    log_trace("TimerQueue::cancelAllInLoop clear the timers_");
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    detail::readTimerfd(timerfd_, now);

    std::vector<std::unique_ptr<Timer>> expireds = getExpired(now);

    cancelingExpiredTimers_.store(true);
    cancelingTimers_.clear();
    for (auto it = expireds.begin(); it != expireds.end(); ++it)
    {
        (*it)->run();
    }
    cancelingExpiredTimers_.store(false);

    // set the next expiration time for timerfd
    reset(expireds, now);
}
std::vector<std::unique_ptr<Timer>> TimerQueue::getExpired(Timestamp now)
{
    log_trace("get in TimerQueue::getExpired and time=%s", now.toString().c_str());
    std::vector<std::unique_ptr<Timer>> expireds;
    std::unique_ptr<Timer> tempPtr(new Timer(now, UINT64_MAX));

    // move the timer to vector
    auto end = timers_.lower_bound(tempPtr);
    for (auto it = timers_.begin(); it != end;)
    {
        auto tempIt = it;
        it++;
        std::unique_ptr<Timer> tempTimerNode = std::move(timers_.extract(tempIt).value());
        expireds.emplace_back(std::move(tempTimerNode));
    }
    return expireds;
}
void TimerQueue::reset(std::vector<std::unique_ptr<Timer>> &expired, Timestamp now)
{
    if (cancelAllTimers_)
    {
        cancelAllTimers_.store(false);
        return;
    }
    Timestamp nextExpire(0);
    //  for interval
    for (int i = 0; i < expired.size(); i++)
    {
        if (expired[i]->repeat() && cancelingTimers_.find(expired[i]->getSequence()) == cancelingTimers_.end())
        {
            // LOG_DEBUG("TimerQueue::reset add repeat timer:%lu and cancelingTimer.size=%d", expired[i]->getSequence(), (int)cancelingTimers_.size());
            // LOG_DEBUG("TimerQueue::reset expired[%d]:%lu cancelingTimers.size=%d", i, expired[i]->getSequence(), (int)cancelingTimers_.size());
            expired[i]->restart(now);
            insert(std::move(expired[i]));
        }
    }

    if (!timers_.empty())
    {
        nextExpire = (*timers_.begin())->getExpirationTime();
    }

    if (nextExpire.valid())
    {
        detail::resetTimerfd(timerfd_, nextExpire);
    }
}
bool TimerQueue::insert(std::unique_ptr<Timer> timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->getExpirationTime();
    auto it = timers_.begin();

    // check that if need to reset the timerfd
    if (it == timers_.end() || when < (*it)->getExpirationTime())
    {
        earliestChanged = true;
    }

    {
        auto result = timers_.insert(std::move(timer));
        assert(result.second);
    }

    return earliestChanged;
}
