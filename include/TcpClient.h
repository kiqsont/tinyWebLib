#ifndef KIQSONT_MUDUO_OVERWRITE_TCPCLIENT
#define KIQSONT_MUDUO_OVERWRITE_TCPCLIENT

#include "TcpConnection.h"

#include <mutex>

class Connector;

class TcpClient : noncopyable
{
public:
    using ConnectorPtr = std::shared_ptr<Connector>;

public:
    TcpClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg = std::string("default_name"));
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lg(mutex_);
        return connection_;
    }

    EventLoop *getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_.store(true); }

    const std::string &name() const
    {
        return name_;
    }

    void setConnectionCallback(ConnectionCallback cb)
    {
        connectionCallback_ = std::move(cb);
    }
    void setMessageCallback(MessageCallback cb)
    {
        messageCallback_ = std::move(cb);
    }
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    {
        writeCompleteCallback_ = std::move(cb);
    }

private:
    void newConnection(int sockfd);

    void removeConnection(const TcpConnectionPtr &conn);

private:
    EventLoop *loop_;
    ConnectorPtr connector_;
    const std::string name_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    std::atomic_bool retry_{false};
    std::atomic_bool connect_{true};

    int nextConnID_ = 1;
    TcpConnectionPtr connection_;
    mutable std::mutex mutex_;
};

#endif // KIQSONT_MUDUO_OVERWRITE_TCPCLIENT