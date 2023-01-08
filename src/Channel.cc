#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

/*
Channel::~Channel(){}
*/
void Channel::handleEvent(Timestamp receiveTime)
{
    LOG_DEBUG("Channel::handleEvent for fd=%d and tie is %d", fd_, (int)tied_);
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
    LOG_INFO("channel handleEvent revents:%d", revents_);

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            LOG_DEBUG("Channel::handleEventWithGuard for closeCallback");
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            LOG_DEBUG("Channel::handleEventWithGuard for errorCallback");
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

    LOG_DEBUG("Channel::handleEventLWithGuard end\n");
}
