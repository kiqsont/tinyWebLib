#ifndef KIQSONT_MUDUO_COPY_EPOLLPOLLER
#define KIQSONT_MUDUO_COPY_EPOLLPOLLER

#include "Poller.h"


#include <vector>
#include <sys/epoll.h>

class EPollPoller :public Poller
{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller();
    
    // To override from Poller
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const; // set active connection
    void update(int operation, Channel* channel); // update channel flag

private:
    using EventList = std::vector<epoll_event>;
    inline static const int kInitEventListSize = 16;

    int epollfd_;
    EventList events_;
    
};

#endif  // KIQSONT_MUDUO_COPY_EPOLLPOLLER

