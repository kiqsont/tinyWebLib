#include <iostream>
#include <string>
#include "Logger.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include <unistd.h>
#include <functional>

using std::cout;
using std::endl;

using namespace asyncLogger;

namespace EntireProject_for_EchoServer
{
    void onConnection(const TcpConnectionPtr &conn)
    {
        std::string connectedFlag = "DOWN";
        if (conn->connected())
        {
            connectedFlag = "UP";
        }
        log_info("Project_Copy Server - %s -> %s is %s", conn->localAddress().toIpPort().c_str(), conn->peerAddress().toIpPort().c_str(), connectedFlag.c_str());
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg(buf->retrieveAllAsString());
        log_info("%s echo %d bytes,content:%s;in %s", conn->name().c_str(), static_cast<int>(msg.size()), msg.c_str(), time.toString().c_str());
        conn->send(msg);
    }

    void test()
    {
        log_debug("use muduo_copy to build a echo server in DEBUG");
        EventLoop loop;
        // std::cout << "thread right?" << loop.isInLoopThread() << std::endl;

        InetAddress listenAddr(8888);
        TcpServer server(&loop, listenAddr, "echo_muduo_copy_server");
        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);
        server.start();

        loop.loop();
    }

}

namespace TimeServer
{
    void onConnection(const TcpConnectionPtr &conn)
    {
        std::string connectedFlag = "DOWN";
        if (conn->connected())
        {
            connectedFlag = "UP";
        }
        log_info("Project_Copy Server - %s -> %s is %s", conn->localAddress().toIpPort().c_str(), conn->peerAddress().toIpPort().c_str(), connectedFlag.c_str());
        conn->send(Timestamp::now().toString());
        if (conn->connected())
        {
            conn->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg(buf->retrieveAllAsString());
        log_info("discarded %d bytes in %s", static_cast<int>(msg.size()), time.toString().c_str());
    }

    void test()
    {
        log_debug("use muduo_copy to build a time server in DEBUG");
        EventLoop loop;
        // std::cout << "thread right?" << loop.isInLoopThread() << std::endl;

        InetAddress listenAddr(8888);
        TcpServer server(&loop, listenAddr, "time_muduo_copy_server");
        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);
        server.setThreadNum(3);
        server.start();

        loop.loop();
    }
}

/*
namespace LoggerTest
{
    void test()
    {
        log_info("info");
        log_debug("debug");
        LOG_ERROR("error");
        std::cout << "flag:";
        int flag;
        std::cin >> flag;
        if (flag == 0)
        {
            LOG_FATAL("fatal");
        }
    }

    void test2()
    {
        std::string str = "abcdeft1324234";
        log_info("for std::string:%s", str.c_str());
        char buf[32]{0};
        for (int i = 0; i < 5; i++)
        {
            buf[i] = (char)('a' + i);
        }
        log_info("char[] buf test:%s%s", buf); // bug
        std::cout << "--------end-----------" << std::endl;
    }
}

namespace ThreadClassTest
{
    void test()
    {
        Thread t([]
                 {
            std::cout << "run a Thread entity : " << std::this_thread::get_id() << std::endl;
            for(int i = 0;i < 3;i++)
            {
                std::cout << "thread task:" << i << std::endl;
                sleep(1);
            } },
                 "a_thread_test");
        t.start();
        std::cout << "thread started? " << t.started() << std::endl;
        std::cout << "thread name:" << t.name() << " and thread id:" << t.getThreadId() << std::endl;
        t.join();
    }
}

#include "InetAddress.h"
#include "Socket.h"
#include <string>
#include <cstdio>
namespace SocketTest_for_Echo
{

    static int createNoneblocking()
    {

        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0)
        {
            LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        }
        return sockfd;
    }

    void test()
    {
        InetAddress listenAddr(8888);
        log_info("InetAddress test:%s", listenAddr.toIpPort().c_str());
        std::cout << "IP:" << listenAddr.toIp() << " Port:" << listenAddr.toPort() << std::endl;
        Socket sock(createNoneblocking());
        sockaddr_in sockaddr = *listenAddr.getSockAddr();
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock.bindAddress(listenAddr);
        sock.listen();
        InetAddress peerAddr;
        int connFd = sock.accept(&peerAddr);
        std::cout << "peerAddr" << peerAddr.toIpPort() << std::endl;
        char buf[128]{0};
        while (1)
        {
            int ret = ::recv(connFd, buf, sizeof buf, 0);
            if (ret < 0)
            {
                LOG_FATAL("recv Error");
            }
            buf[ret] = 0;
            std::string check(buf);
            log_info("recv check1:%s", check.c_str());
            if (check.substr(0, 4) == "exit")
                break;
        }
        ::close(connFd);
    }

}

namespace EventLoop_EchoServer
{

    static int createNoneblocking()
    {

        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0)
        {
            LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        }
        return sockfd;
    }

    void onMessageCallback();

    void newConnection(int sockfd, const InetAddress &peerAddr, EventLoop *loop)
    {
        log_info("newConnection from [%s]", peerAddr.toIpPort().c_str());
    }

    void tryAccept(Socket &listenFd, EventLoop *loop)
    {

        InetAddress peeraddr;
        int connfd = listenFd.accept(&peeraddr);
        if (connfd < 0)
        {
            LOG_ERROR("accpet error");
        }
        else
        {
            newConnection(connfd, peeraddr, loop);
        }
    }

    void test()
    {
        EventLoop loop;
        InetAddress listenAddr(8888);
        Socket listenFd(createNoneblocking());
        listenFd.setReuseAddr(true);
        listenFd.setResuePort(true);
        listenFd.bindAddress(listenAddr);
        Channel listenChannel(&loop, listenFd.fd());
        listenChannel.setReadCallback(std::bind(tryAccept, listenFd, &loop));

        listenFd.listen();
        listenChannel.enableReading();

        loop.loop();
    }
}
*/
int main()
{
    // BufferTest::test();
    // SocketTest_for_Echo::test();
    //   ThreadClassTest::test();
    //  LoggerTest::test2();
    // EntireProject_for_EchoServer::test();
    TimeServer::test();
}
