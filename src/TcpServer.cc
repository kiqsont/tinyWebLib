#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

#include <cstring>
#include <unistd.h>

using namespace asyncLogger;

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        log_fatal("{}:{}:{}  mainLoop is null \n", __FILE__, __FUNCTION__, __LINE__);
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
    SSL_CTX_free(ctx_);
}

// acceptor callback this
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = threadPool_->getNextLoop(); // select a loop to handle the new connetcion
    char buf[64]{0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    log_trace("TcpServer::newConnection [{}] - new connection [{}] from {} \n", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    sockaddr_in local;
    memset(&local, 0, sizeof local);
    socklen_t addrLen = sizeof local;
    if (::getsockname(sockfd, (sockaddr *)&local, &addrLen) < 0)
    {
        log_error("sockets::getLoaclAddr\n");
    }
    InetAddress localAddr(local);

    SSL *ssl = nullptr;
    if (security_)
    {
        ssl = SSL_new(ctx_);
        SSL_set_fd(ssl, sockfd);
        // ensure connection
        int code = 0, retryTimes = 0;
        while ((code = SSL_accept(ssl)) <= 0 && retryTimes++ < 100)
        {
            if (SSL_get_error(ssl, code) != SSL_ERROR_WANT_READ)
            {
                log_error("ssl accept error {}", SSL_get_error(ssl, code));
                break;
            }
            usleep(50);
        }
        if (code != 1)
        {
            log_error("ssl accept error {} and clean the connection", SSL_get_error(ssl, code));
            SSL_free(ssl);
            ::close(sockfd);
            return;
        }
    }

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr, security_, ssl));
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

bool TcpServer::setSecurity(const std::string &cacertPath, const std::string &privkeyPath)
{
    if (started_.load() != 0 || security_.load() != false)
        return false;

    log_trace("Try to set TLS");

    security_.store(true);
    ctx_ = SSL_CTX_new(SSLv23_server_method());
    if (nullptr == ctx_)
    {
        log_error("ctx err in SSL_CTX_new");
        return false;
    }

    if (SSL_CTX_use_certificate_file(ctx_, cacertPath.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        log_error("load cacert.pem err in SSL_CTX_use_certificate_file");
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx_, privkeyPath.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        log_error("load privkey.pem err in SSL_CTX_use_PrivateKey_file");
        return false;
    }

    if (!SSL_CTX_check_private_key(ctx_))
    {
        log_error("private key err in SSL_CTX_check_private_key");
        return false;
    }
    return true;
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    log_trace("TcpServer::removeConnectionInLoop [{}] - connection {}\n", name_.c_str(), conn->name().c_str());
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
        log_trace("TcpServer::setMaxConnectionTime Failed, server has started");
        return;
    }
    enableTimingWheel_.store(true);
    connectionBuckets_ = std::make_shared<timingWheel::WeakConnectionList>(maxTime);
    loop_->runEvery(1.0, std::bind(&TcpServer::onTimingWheel, this));
}