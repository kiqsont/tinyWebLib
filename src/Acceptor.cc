#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
using namespace asyncLogger;

static int createNoneblocking()
{

    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        fatal("{}:{}:{} listen socket create err:{} \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop), acceptSocket_(createNoneblocking()), acceptChannel_(loop, acceptSocket_.fd())
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setResuePort(reuseport);
    // the above like         sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

// when new connection
void Acceptor::handleRead()
{
    InetAddress peeraddr;
    int connfd = acceptSocket_.accept(&peeraddr);
    debug("Acceptor::handleRead and peerAddr:{} and connfd={}", peeraddr.toIpPort().c_str(), connfd);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peeraddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        error("{}:{}:{} accept err:{} \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            error("{}:{}:{} sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}

void Acceptor::listen()
{
    debug("Acceptor::listen begin to listen");
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}