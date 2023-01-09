
#ifndef KIQSONT_HTTP_RESPONSE
#define KIQSONT_HTTP_RESPONSE

#include <map>
#include <string>
#include <memory>
#include "Buffer.h"
#include "HttpRequest.h"

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        kUnkonw,
        k200ok = 200,
        k301MovePermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

public:
    explicit HttpResponse(bool close)
        : closeConnection_(close) {}

    void setStatusCode(HttpStatusCode code)
    {
        statusCode_ = code;
    }

    void setStatusMessage(const std::string &msg)
    {
        statusMessage_ = msg;
    }

    void setCloseConnection(bool on)
    {
        closeConnection_ = on;
    }

    bool closeConnection() const
    {
        return closeConnection_;
    }

    void setContentType(const std::string &contentType)
    {
        addHeader("Content-Type", contentType);
    }

    void addHeader(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    void setBody(const std::string &body)
    {
        body_ = body;
    }

    void appendToBuffer(Buffer *output);

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode statusCode_ = kUnkonw;
    std::string statusMessage_;
    bool closeConnection_;
    std::string body_;
    HttpRequest::Version version_;
};

#endif // KIQSONT_HTTP_RESPONSE