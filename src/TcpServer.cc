#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

#include <cstring>

using namespace asyncLogger;

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        fatal("{}:{}:{}  mainLoop is null \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, InetAddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)), threadPool_(new EventLoopThreadPool(loop, name_)), connectionCallback_(TcpConnection::muduoDefaultConnectionCallback), messageCallback_(TcpConnection::muduoDefaultMessageCallback)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    /*
    for(auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestoryed,conn));
    }
    */

    for (auto &[name, ptr] : connections_)
    {
        TcpConnectionPtr conn(ptr);
        ptr.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    }
}

// acceptor callback this
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = threadPool_->getNextLoop(); // select a loop to handle the new connetcion
    char buf[64]{0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    trace("TcpServer::newConnection [{}] - new connection [{}] from {} \n", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    sockaddr_in local;
    memset(&local, 0, sizeof local);
    socklen_t addrLen = sizeof local;
    if (::getsockname(sockfd, (sockaddr *)&local, &addrLen) < 0)
    {
        error("sockets::getLoaclAddr\n");
    }
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;

    // user set
    conn->setConnectionCallback(std::bind(&TcpServer::aopForConntionCallback, this, std::placeholders::_1));
    // conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(std::bind(&TcpServer::aopForMessageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    // LOG_DEBUG("TcpServer::start try to start");
    if (started_++ == 0) // prevent called by many threads
    {
        // LOG_DEBUG("TcpServer::start get in start");
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    trace("TcpServer::removeConnectionInLoop [{}] - connection {}\n", name_.c_str(), conn->name().c_str());
    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}

void TcpServer::aopForConntionCallback(const TcpConnectionPtr &conn)
{
    // for timing wheel

    if (enableTimingWheel_.load())
    {
        timingWheel::connectionTimingWheel(conn, connectionBuckets_);
    }

    connectionCallback_(conn);
}
void TcpServer::aopForMessageCallback(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{

    if (enableTimingWheel_.load())
    {
        timingWheel::messageTimingWheel(conn, connectionBuckets_);
    }

    messageCallback_(conn, buf, receiveTime);
}

void TcpServer::onTimingWheel()
{
    connectionBuckets_->push_back(timingWheel::Bucket());
}

void TcpServer::setMaxConnectionTime(int maxTime)
{
    if (started_ != 0)
    {
        trace("TcpServer::setMaxConnectionTime Failed, server has started");
        return;
    }
    enableTimingWheel_.store(true);
    connectionBuckets_ = std::make_shared<timingWheel::WeakConnectionList>(maxTime);
    loop_->runEvery(1.0, std::bind(&TcpServer::onTimingWheel, this));
}