#ifndef KIQSONT_MUDUO_COPY_SOCKET
#define KIQSONT_MUDUO_COPY_SOCKET

#include "noncopyable.h"

class InetAddress;

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd)
    {
    }
    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setResuePort(bool on);
    void setKeepAlive(bool on);

    static int createNoneblockingFD();
    static int getSocketError(int sockfd);
    static bool isSelfConnect(int sockfd);

private:
    const int sockfd_;
};

#endif // KIQSONT_MUDUO_COPY_SOCKET