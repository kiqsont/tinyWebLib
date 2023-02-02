#ifndef KIQSONT_MUDUO_OVERWRITE_INETADDRESS
#define KIQSONT_MUDUO_OVERWRITE_INETADDRESS

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class InetAddress // copyable
{
public:
    InetAddress() = default;

    explicit InetAddress(uint16_t port, std::string ip = "0.0.0.0");

    explicit InetAddress(const sockaddr_in &addr);

    std::string toIp() const;

    uint16_t toPort() const;

    std::string toIpPort() const;

    const sockaddr_in *getSockAddr() const { return &addr_; }

    void setSockaddr(const sockaddr_in &addr) { addr_ = addr; }

public:
    static sockaddr_in getLocalAddr(int sockfd);
    static sockaddr_in getPeerAddr(int sockfd);

private:
    sockaddr_in addr_;
};

#endif // KIQSONT_MUDUO_OVERWRITE_INETADDRESS
