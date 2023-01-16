#ifndef KIQSONT_HTTP_SERVER
#define KIQSONT_HTTP_SERVER

#include "TcpServer.h"
#include "Logger.h"
#include "noncopyable.h"
#include <map>

#include "HttpResponse.h"
#include "HttpContext.h"
#include "HttpRequest.h"

using namespace asyncLogger;

class HttpServer : noncopyable
{
public:
    using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;
    using PathMap = std::map<std::string, HttpCallback>;

public:
    HttpServer(EventLoop *loop, InetAddress &listenAddr, const std::string &name, TcpServer::Option option = TcpServer::kNoReusePort);
    EventLoop *getLoop() const
    {
        return server_.getLoop();
    }

    void setHttpCallback(const HttpCallback &cb)
    {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }

    void start()
    {
        log_info("HttpsServer[{}] starts listening on {}", server_.name(), server_.ipPort());
        server_.start();
    }

    bool setSecurity(const std::string &cacertPath, const std::string &privkeyPath)
    {
        return server_.setSecurity(cacertPath, privkeyPath);
    }

    void Get(const std::string &path, HttpCallback cb);
    void Post(const std::string &path, HttpCallback cb);
    void File(const std::string &path, std::string filename);
    void setStaticDir(const std::string &staticDir);

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr &, const HttpRequest &);
    bool sendStaticFile(const std::string &filename, const TcpConnectionPtr &conn);

private:
    TcpServer server_;
    HttpCallback httpCallback_;
    PathMap getMap_;
    PathMap postMap_;
    std::map<std::string, std::string> staticFiles_;
    std::string staticDir_ = "../static/";
};

#endif // KIQSONT_HTTP_SERVER