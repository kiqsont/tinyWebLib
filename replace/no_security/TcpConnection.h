#ifndef KIQSONT_MUDUO_COPY_TCPCONNECTION
#define KIQSONT_MUDUO_COPY_TCPCONNECTION

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <cstring>
#include <atomic>
#include <any>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    static void muduoDefaultConnectionCallback(const TcpConnectionPtr &conn);
    static void muduoDefaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buf, Timestamp);

public:
    TcpConnection(EventLoop *loop, const std::string &nameArg, int sockfd, const InetAddress &loaclAddr, const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    void send(const std::string &msg);

    void send(const void *data, size_t len);

    void shutdown();

    void foreClose();

    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb)
    {
        highWaterMarkCallback_ = cb;
    }
    void setCloseCallback(const CloseCallback &cb)
    {
        closeCallback_ = cb;
    }

    void setContext(const std::any &context)
    {
        context_ = context;
    }
    const std::any &getContext() const
    {
        return context_;
    }

    std::any &getMutableContext()
    {
        return context_;
    }

    void connectEstablished();
    void connectDestoryed();

private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void setState(StateE state) { state_.store(state); }
    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();
    void foreCloseInLoop();

private:
    EventLoop *loop_; // In usual, it could not be baseLoop/mainLoop,TcpConnection runs in SubReactor
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    mutable std::any context_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;

    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBUffer_;
};

#endif // KIQSONT_MUDUO_COPY_TCPCONNECTION
