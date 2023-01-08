#include "Poller.h"
#include "EPollPoller.h"

#include <cstdlib>

Poller* Poller::newDefaultPoll(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        return new EPollPoller(loop);
    }
}
