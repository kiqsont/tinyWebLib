#ifndef KIQSONT_MUDUO_OVERWRITE_CHANNEL
#define KIQSONT_MUDUO_OVERWRITE_CHANNEL

// 封装sockfd去处理相关的event(EPOLLIN/EPOLLOUT)

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

public:
    Channel(EventLoop *loop, int fd);

    //~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { readCallback_ = cb; }
    void setWriteCallback(EventCallback cb) { writeCallback_ = cb; }
    void setCloseCallbcak(EventCallback cb) { closeCallback_ = cb; }
    void setErrorCallback(EventCallback cb) { errorCallback_ = cb; }

    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() const { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop *ownerLoop() { return loop_; }

    void remove();

private:
    void update();

    void handleEventWithGuard(Timestamp receiveTime);

private:
    inline static constexpr int kNoneEvent = 0;
    inline static constexpr int kReadEvent = EPOLLIN | EPOLLPRI;
    inline static constexpr int kWriteEvent = EPOLLOUT;

    EventLoop *loop_;
    const int fd_; // for Poller
    int events_;   // register events
    int revents_;  // events poller return
    int index_;    // for ctl

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

#endif // KIQSONT_MUDUO_OVERWRITE_CHANNEL
