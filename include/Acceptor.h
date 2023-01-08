#ifndef KIQSONT_MUDUO_COPY_ACCEPTOR
#define KIQSONT_MUDUO_COPY_ACCEPTOR

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

public:
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport = true);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

    void listen();

    bool listenning() const { return listenning_; }

private:
    void handleRead();

private:
    EventLoop *loop_; // baseLoop/mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_ = false;
};

#endif // KIQSONT_MUDUO_COPY_ACCEPTOR