#ifndef KIQSONT_MUDUO_COPY_POLLER
#define KIQSONT_MUDUO_COPY_POLLER

#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

// Poller:for poll/epoll and manage channels

class Poller
{
public:
    using ChannelList = std::vector<Channel *>;

public:
    Poller(EventLoop *);

    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoll(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;
    EventLoop *ownerLoop_;
};

#endif // KIQSONT_MUDUO_COPY_POLLER
