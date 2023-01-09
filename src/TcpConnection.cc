#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <unistd.h>
#include <errno.h>

using namespace asyncLogger;

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        log_fatal("{}:{}:{}  TcpConnection Loop is null \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &nameArg, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr, bool isSSL, SSL *ssl)
    : loop_(CheckLoopNotNull(loop)), name_(nameArg), state_(kConnecting), reading_(true), socket_(new Socket(sockfd)), channel_(new Channel(loop, sockfd)), localAddr_(localAddr), peerAddr_(peerAddr), security_(isSSL), ssl_(ssl), highWaterMark_(64 * 1024 * 1024) // 64M
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallbcak(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    log_trace("TcpConnection::ctor[{}] at fd={}\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    log_trace("TcpConnection::dtor[{}] at fd={} state={}\n", name_.c_str(), channel_->fd(), static_cast<int>(state_));
    if (security_.load())
    {
        log_trace("clean ssl");
        socket_.reset();
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
    }
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    long n;
    if (security_)
        n = inputBuffer_.readSSL(ssl_, &saveErrno);
    else
        n = inputBuffer_.readFd(channel_->fd(), &saveErrno);

    if (n > 0)
    {
        // EPOLLIN onMessage
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        log_error("TcpConnection::handleRead\n");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting())
    {
        int saveErrno = 0;
        long n;
        if (security_)
            n = outputBUffer_.writeSSL(ssl_, &saveErrno);
        else
            n = outputBUffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0)
        {
            outputBUffer_.retrieve(n);
            if (outputBUffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            log_error("TcpConnection::handleWrite with errno:{}", saveErrno);
        }
    }
    else
    {
        log_error("TcpConnection fd ={} id down,no more writing\n", channel_->fd());
    }
}
void TcpConnection::handleClose()
{
    log_trace("TcpConnection::handleClose fd={} state={} \n", channel_->fd(), static_cast<int>(state_.load()));
    setState(StateE::kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    log_error("TcpConnection::handleErrno name:{} - SO_ERROR:{} \n", name_.c_str(), err);
}

void TcpConnection::send(const std::string &msg)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(msg.c_str(), msg.size());
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, msg.c_str(), msg.size()));
        }
    }
}

void TcpConnection::send(const void *data, size_t len)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(data, len);
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, data, len));
        }
    }
}

// send message
void TcpConnection::sendInLoop(const void *message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected)
    {
        log_error("disconnected, give up writing\n");
        return;
    }

    // the first time to write and out buffer hasn't data to send
    if (!channel_->isWriting() && outputBUffer_.readableBytes() == 0)
    {
        if (security_)
            nwrote = SSL_write(ssl_, message, len);
        else
            nwrote = ::write(channel_->fd(), message, len);

        if (nwrote >= 0)
        {

            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                // send all data, don't need to set EPOLLOUT to channel
                // or it would call handleWrite
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nworte < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                log_error("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE RESET
                {
                    faultError = true;
                }
            }
        }
    }

    // data hasn't sent all ,it need to be save in outputBuffer
    // and set EPOLLOUT to channel, calls handleWrite
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBUffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBUffer_.append((char *)message + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting(); // set EPOLLOUT
        }
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); // channel -> EPOLLIN

    // new connection callback
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestoryed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting()) // channel has send all output data
    {
        socket_->shutdownWrite(); // it would trigger EPOLLHUP and callback Channel::closeCallback
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::foreClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnected);
        loop_->queueInLoop(std::bind(&TcpConnection::foreCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::foreCloseInLoop()
{
    handleClose();
}

void TcpConnection::muduoDefaultConnectionCallback(const TcpConnectionPtr &conn)
{
    const char *flag = conn->connected() ? "UP" : "DOWN";
    log_info("{} -> {} is {}", conn->localAddress().toIpPort().c_str(), conn->peerAddress().toIpPort().c_str(), flag);
}

void TcpConnection::muduoDefaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    buf->retrieveAll();
}