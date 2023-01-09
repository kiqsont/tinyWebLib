#include "TcpServer.h"
#include "EventLoop.h"
#include "Logger.h"

using namespace asyncLogger;

void onConnect(const TcpConnectionPtr &conn)
{
    std::string connectedFlag = "DOWN";
    if (conn->connected())
    {
        connectedFlag = "UP";
        conn->send(Timestamp::now().toString());
        conn->shutdown();
    }
    log_info("Project_Copy Server - {} -> {} is {}", conn->localAddress().toIpPort().c_str(), conn->peerAddress().toIpPort().c_str(), connectedFlag.c_str());
}

int main()
{
    EventLoop loop;
    InetAddress serverAddr(8888);
    TcpServer server(&loop, serverAddr, "time server");
    server.setConnectionCallback(onConnect);
    server.start();
    loop.loop();
}