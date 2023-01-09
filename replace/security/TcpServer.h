#ifndef KIQSONT_MUDUO_COPY_TCPSERVER
#define KIQSONT_MUDUO_COPY_TCPSERVER

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "TimingWheel.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };

public:
    TcpServer(EventLoop *loop, InetAddress &listenAddr, const std::string &nameArg, Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads);

    void start(); // to listen

    void setMaxConnectionTime(int maxTime);

    bool setSecurity(const std::string &cacertPath, const std::string &privkeyPath);

    EventLoop *getLoop() const
    {
        return loop_;
    }

    const std::string &name() const
    {
        return name_;
    }

    const std::string &ipPort() const
    {
        return ipPort_;
    }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    void aopForConntionCallback(const TcpConnectionPtr &conn);
    void aopForMessageCallback(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime);
    void onTimingWheel();

private:
    EventLoop *loop_; // baseLoop / mainLoop

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;              // mainLoop acceptor to handle new connection
    std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;

    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_{0};

    int nextConnId_ = 1;
    ConnectionMap connections_; // all connecions

    std::shared_ptr<timingWheel::WeakConnectionList> connectionBuckets_;
    std::atomic_bool enableTimingWheel_{false};

    std::atomic_bool security_ = false;
    SSL_CTX *ctx_;
};

#endif // KIQSONT_MUDUO_COPY_TCPSERVER
