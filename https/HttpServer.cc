#include "HttpServer.h"
#include "TcpConnection.h"
#include <functional>
#include <fcntl.h>
#include <unistd.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace detail
{
    void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
    {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setBody("404");
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

HttpServer::HttpServer(EventLoop *loop, InetAddress &listenAddr, const std::string &name, TcpServer::Option option)
    : server_(loop, listenAddr, name, option), httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    log_debug("HttpServer::onConnection get connection");
    if (conn->connected())
    {
        conn->setContext(HttpContext());
    }
}
void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    log_debug("HttpServer::onMessage get a message");
    HttpContext context = std::any_cast<HttpContext>(conn->getContext());
    log_debug("HttpServer::onMessage try to parse Request");
    if (!context.parseRequest(buf, receiveTime))
    {
        log_debug("HttpServer::onMessage after parseRequest");
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context.gotAll())
    {
        log_debug("HttpServer::onMessage call for onRequest");

        onRequest(conn, context.request());
        context.reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const std::string &path = req.path();
    // check file get
    if (req.method() == HttpRequest::kGet && staticFiles_.find(path) != staticFiles_.end())
    {
        sendStaticFile(staticFiles_[path], conn);
        return;
    }

    const std::string &connection = req.getHeader("Connection");
    bool close = connection == "close" || (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);

    // POST
    auto itPost = postMap_.find(path);
    if (req.method() == HttpRequest::kPost && itPost != postMap_.end())
    {
        itPost->second(req, &response);
    }
    auto itGet = getMap_.find(path);
    if (req.method() == HttpRequest::kGet && itGet != getMap_.end())
    {
        itGet->second(req, &response);
    }

    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    std::string temp = buf.retrieveAllAsString();
    conn->send(temp);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

void HttpServer::Get(const std::string &path, HttpCallback cb)
{
    getMap_[path] = std::move(cb);
}

void HttpServer::Post(const std::string &path, HttpCallback cb)
{
    postMap_[path] = std::move(cb);
}

void HttpServer::File(const std::string &path, std::string filename)
{
    staticFiles_[path] = std::move(filename);
}

bool HttpServer::sendStaticFile(const std::string &filename, const TcpConnectionPtr &conn)
{
    std::string fullfilename = "static/" + filename;
    int fd = open(fullfilename.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        conn->send("open " + filename + " failed");
        log_error("open file {} error", filename);
        return false;
    }

    char buf[1024]{0};
    ssize_t n = 0;
    while (1)
    {
        n = read(fd, buf, sizeof buf);
        if (0 <= n)
            break;
        conn->send(buf, n);
    }
    conn->shutdown();
    close(fd);
    return true;
}

void HttpServer::setStaticDir(const std::string &staticDir)
{
    staticDir_ = staticDir;
    if (staticDir.back() != '/')
        staticDir_.push_back('/');
}