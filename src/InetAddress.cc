#include "InetAddress.h"
#include "Logger.h"

#include <cstring>

using namespace asyncLogger;

sockaddr_in InetAddress::getLocalAddr(int sockfd)
{
    sockaddr_in localaddr{0};
    socklen_t addrlen = sizeof localaddr;
    if (::getsockname(sockfd, (sockaddr *)&localaddr, &addrlen))
    {
        log_error("Socket::getLocalAddr");
    }
    return localaddr;
}

sockaddr_in InetAddress::getPeerAddr(int sockfd)
{
    sockaddr_in peeraddr{0};
    socklen_t addrlen = sizeof peeraddr;
    if (::getpeername(sockfd, (sockaddr *)&peeraddr, &addrlen))
    {
        log_error("Socket::getLocalAddr");
    }
    return peeraddr;
}

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in &addr)
    : addr_(addr)
{
}

std::string InetAddress::toIp() const
{
    char buf[64]{0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ::ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const
{
    char buf[64]{0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    uint16_t port = ntohs(addr_.sin_port);
    size_t end = strlen(buf);
    sprintf(buf + end, ":%u", port);
    return buf;
}
