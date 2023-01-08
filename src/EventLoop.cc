#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"
#include "TimerQueue.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

using namespace asyncLogger;

// a loop just in a thread
thread_local EventLoop *t_loopInThisThread = nullptr;

// default poller timeout value
const int kPollTimeMs = 10000;

// create wakeup fd to notify subReactor's channel
static int createEvent()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        log_fatal("eventfd error:{} \n", errno);
    }
    return evtfd;
}

unsigned long EventLoop::get_thread_id()
{
    unsigned long id;
    memcpy(&id, &threadId_, 8);
    return id;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), callingPendingFunctors_(false), threadId_(std::this_thread::get_id()), poller_(Poller::newDefaultPoll(this)), wakeupFd_(createEvent()), wakeupChannel_(new Channel(this, wakeupFd_)), timerQueue_(new TimerQueue(this))
{
    // log_trace("EventLoop created {} in thread {} \n", this, get_thread_id());
    if (t_loopInThisThread)
    {
        // log_fatal("Another EventLoop {} exists in this thread {} \n", t_loopInThisThread, get_thread_id());
        log_fatal("Another EventLoop exists in this thread {} \n", get_thread_id());
    }
    else
    {
        t_loopInThisThread = this;
    }

    // set wakeup's events and callback
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading(); // every eventloop and listen the EPOLLIN for wakeupChannel
    log_trace("EventLoop ctor end");
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        log_error("EventLoop::handleRead() reads {} bytes instead of 8", n);
    }
}

void EventLoop::loop()
{
    looping_.store(true);
    quit_.store(false);

    // log_trace("EventLoop {} start looping \n", this);
    while (!quit_.load())
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel *channel : activeChannels_)
        {
            log_trace("EventLoop::loop get in channelHandle fd={}", channel->fd());
            channel->handleEvent(pollReturnTime_);
        }

        doPendingFunctors(); // callbacks after events
    }
    // log_trace("EventLoop {} stop looping \n", this);
    looping_.store(false);
}

void EventLoop::quit()
{
    quit_.store(true);

    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    int inThread = isInLoopThread();
    log_trace("EventLoop::runInLoop in loopThread:{}", inThread);
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lg(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // notify the thread which need to run cb
    if (!isInLoopThread() || callingPendingFunctors_.load())
    {
        /*
            why callingPendingFunctors_.load()
            if doPendingFunctors() running,callingPendingFunctors is true
            but after doPendingFunctors(), watch line88 ,it would be blocking in the ::epoll_wait
        */

        wakeup();
    }
}

TimerID EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}
TimerID EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp time(Timestamp::afterNow(delay));
    return runAt(time, std::move(cb));
}
TimerID EventLoop::runEvery(double interval, TimerCallback cb)
{
    Timestamp time(Timestamp::afterNow(interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}
void EventLoop::cancel(TimerID timerID)
{
    timerQueue_->cancel(timerID);
}

void EventLoop::cancelAll()
{
    timerQueue_->cancelAll();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        log_error("EventLoop::wakeup() writes {} bytes instand of 8 \n", n);
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{

    log_trace("EventLoop::doPendingFunctors callback");

    std::vector<Functor> functors;
    callingPendingFunctors_.store(true);

    {
        std::lock_guard<std::mutex> lg(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors)
    {
        functor();
    }

    callingPendingFunctors_.store(false);
}
