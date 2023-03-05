
#include "EventLoop.h"
#include "TcpServer.h"

#define LOG_DEBUG
#include "Logger.h"

using namespace asyncLogger;

void onConnect(const TcpConnectionPtr &conn)
{
    std::string connectedFlag = "DOWN";
    if (conn->connected())
    {
        connectedFlag = "UP";
    }
    log_info("Echo TimingWheel Server - {} -> {} is {}", conn->localAddress().toIpPort().c_str(), conn->peerAddress().toIpPort().c_str(), connectedFlag.c_str());
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg(buf->retrieveAllAsString());
    log_info("{} echo {} bytes,content:{};in {}", conn->name().c_str(), static_cast<int>(msg.size()), msg.c_str(), time.toString().c_str());
    conn->send(msg);
}

int main()
{
    EventLoop loop;
    InetAddress serverAddr(8888);
    TcpServer server(&loop, serverAddr, "echo server");
    server.setMaxConnectionTime(3); // 3s
    server.setConnectionCallback(onConnect);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}