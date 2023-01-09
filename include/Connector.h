#ifndef KIQSONT_MUDUO_OVERWRITE_CONNECTOR
#define KIQSONT_MUDUO_OVERWRITE_CONNECTOR

#include "noncopyable.h"
#include "InetAddress.h"

#include <functional>
#include <memory>
#include <atomic>

class Channel;
class EventLoop;

class Connector : noncopyable, public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

public:
    Connector(EventLoop *loop, const InetAddress &serverAddr);
    ~Connector() noexcept;

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

    void start();
    void restart();
    void stop();

    bool connected() const
    {
        return connect_.load() && state_.load() != kDisconnected;
    }

    const InetAddress &serverAddress() const
    {
        return serverAddr_;
    }

private:
    enum States
    {
        kDisconnected,
        kConnecting,
        kConnected
    };

    inline static const int kMaxRetryDelayMs = 30 * 1000;
    inline static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_.store(s); }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

private:
    EventLoop *loop_;
    InetAddress serverAddr_;
    std::atomic_bool connect_{false};
    std::atomic_int state_{kDisconnected};
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_ = kInitRetryDelayMs;
};

#endif // KIQSONT_MUDUO_OVERWRITE_CONNECTOR