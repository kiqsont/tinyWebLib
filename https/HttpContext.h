#ifndef KIQSONT_HTTP_CONTEXT
#define KIQSONT_HTTP_CONTEXT

#include "HttpRequest.h"

class Buffer;

class HttpContext
{
public:
    enum HttpRequestParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll
    };

public:
    bool parseRequest(Buffer *buf, Timestamp receiveTime);

    bool gotAll() const
    {
        return state_ == kGotAll;
    }

    void reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest dump;
        request_.swap(dump);
    }

    const HttpRequest &request() const
    {
        return request_;
    }

    HttpRequest &request()
    {
        return request_;
    }

private:
    bool processRequestLine(const char *start, const char *end);

    HttpRequestParseState state_;
    HttpRequest request_;
};

#endif // KIQSONT_HTTP_CONTEXT