#include "TcpClient.h"

#include "Connector.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Logger.h"

using namespace asyncLogger;

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        log_fatal("{}:{}:{}  TcpConnection Loop is null \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

namespace detail
{
    void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn)
    {
        loop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    }
}

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg)
    : loop_(CheckLoopNotNull(loop)), connector_(std::make_shared<Connector>(loop, serverAddr)), name_(nameArg), connectionCallback_(TcpConnection::muduoDefaultConnectionCallback), messageCallback_(TcpConnection::muduoDefaultMessageCallback)
{
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient()
{
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lg(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn)
    {
        CloseCallback cb = std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique)
        {
            conn->foreClose();
        }
    }
    else
    {
        connector_->stop();
    }
}

void TcpClient::connect()
{
    if (connector_->connected())
        return;
    connect_.store(true);
    connector_->start();
}
void TcpClient::disconnect()
{
    connect_.store(false);

    {
        std::lock_guard<std::mutex> lg(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}
void TcpClient::stop()
{
    connect_.store(false);
    connector_->stop();
}
void TcpClient::newConnection(int sockfd)
{
    InetAddress peerAddr(InetAddress::getPeerAddr(sockfd));
    char buf[32]{0};
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnID_);
    ++nextConnID_;
    std::string connName = name_ + buf;

    InetAddress localAddr(InetAddress::getLocalAddr(sockfd));
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lg(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}
void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
    {
        std::lock_guard<std::mutex> lg(mutex_);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    if (retry_.load() && connect_.load())
    {
        log_trace("TcpClient::connect[{}] - Reconnecting to [{}]", name_, connector_->serverAddress().toIpPort());
        connector_->restart();
    }
}