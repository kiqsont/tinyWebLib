#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

using namespace asyncLogger;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

/*
Channel::~Channel(){}
*/
void Channel::handleEvent(Timestamp receiveTime)
{
    debug("Channel::handleEvent for fd={} and tie is {}", fd_, (int)tied_);
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// when a new TcpConnection -> channel,it needs a weakptr to record the TcpConnection if alive
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::update()
{
    // update loop->poller
    loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    trace("channel handleEvent revents:{}", revents_);

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            debug("Channel::handleEventWithGuard for closeCallback");
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            debug("Channel::handleEventWithGuard for errorCallback");
            errorCallback_();
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }

    trace("Channel::handleEventLWithGuard end\n");
}
