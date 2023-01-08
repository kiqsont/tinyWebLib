#include "Connector.h"

#include "Logger.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"

#include <error.h>
#include <assert.h>
#include <unistd.h>

using namespace asyncLogger;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop), serverAddr_(serverAddr)
{
    log_debug("Connector::ctor");
}

Connector::~Connector() noexcept
{
    log_debug("Connector::dtor");
}

void Connector::start()
{
    // TODO need to check the connect_ for loop_
    connect_.store(true);
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}
void Connector::restart()
{
    setState(kConnecting);
    retryDelayMs_ = kInitRetryDelayMs;
    start();
}
void Connector::stop()
{
    connect_.store(false);
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
    loop_->cancelAll();
}

void Connector::startInLoop()
{
    if (connect_)
    {
        connect();
    }
    else
    {
        log_info("Connector::startInLoop don't connect");
    }
}
void Connector::stopInLoop()
{
    if (state_.load() == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = Socket::createNoneblockingFD();
    socklen_t len = sizeof(*serverAddr_.getSockAddr());
    int ret = ::connect(sockfd, (const sockaddr *)serverAddr_.getSockAddr(), len);
    int saveErrno = (ret == 0) ? 0 : errno;
    switch (saveErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        log_error("Connect error in Connector::startInLoop:{}", saveErrno);
        ::close(sockfd);
        break;

    default:
        log_trace("Unexpected error in Connector::startInLoop:{}", saveErrno);
        ::close(sockfd);
        break;
    }
}
void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    assert(!channel_);

    channel_.reset();
    channel_ = std::make_unique<Channel>(loop_, sockfd);
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));
    channel_->enableWriting();
}

// socket writeable doesn't mean connect
void Connector::handleWrite()
{
    log_trace("Connector::handleWrite");

    if (state_.load() == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = Socket::getSocketError(sockfd);
        if (err)
        {
            log_warn("Connector::handleWrite - SO_ERROR={}", err);
            retry(sockfd);
        }
        else if (Socket::isSelfConnect(sockfd))
        {
            log_warn("Connector::handleWrite - Self connect");
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                if (newConnectionCallback_)
                    newConnectionCallback_(sockfd);
                else
                    ::close(sockfd);
            }
            else
            {
                ::close(sockfd);
            }
        }
    }
    else
    {
        assert(state_.load() == kDisconnected);
    }
}
void Connector::handleError()
{
    log_error("Connector::handleError state={}", static_cast<int>(state_.load()));
    if (state_.load() == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = Socket::getSocketError(sockfd);
        log_debug("SO_ERROR = {}", err);
        retry(sockfd);
    }
}
void Connector::retry(int sockfd)
{
    ::close(sockfd);
    setState(kDisconnected);
    if (connect_)
    {
        log_debug("Connector::retry - Retry connecting to {} in {} milliseconds", serverAddr_.toIpPort().c_str(), retryDelayMs_);
        loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        log_debug("Connector::retry don't connect");
    }
}
int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}
void Connector::resetChannel()
{
    channel_.reset();
}