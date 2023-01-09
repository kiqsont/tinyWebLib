

#include "HttpContext.h"
#include "Logger.h"

#include "Buffer.h"
#include <algorithm>

using namespace asyncLogger;

bool HttpContext::parseRequest(Buffer *buf, Timestamp receiveTime)
{
    log_debug("HttpContext::parseRequest running");
    bool ok = true;
    bool hasMore = true;
    while (hasMore)
    {
        // requestLine
        if (state_ == kExpectRequestLine)
        {
            log_debug("HttpContext RequestLine");
            const char *crlf = buf->findCRLF();
            if (crlf)
            {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok)
                {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieve(crlf - buf->peek() + 2);
                    state_ = kExpectHeaders;
                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                hasMore = false;
            }
        }
        // for headers
        else if (state_ == kExpectHeaders)
        {
            log_debug("HttpContext RequestHeaders");
            const char *crlf = buf->findCRLF();
            if (crlf)
            {
                const char *colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf)
                {
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    // has request body
                    if (request_.method() == HttpRequest::kPost)
                    {
                        state_ = kExpectBody;
                    }
                    else
                    {
                        state_ = kGotAll;
                        hasMore = false;
                    }
                }
                buf->retrieve(crlf - buf->peek() + 2);
            }
        }
        else if (state_ == kExpectBody)
        {
            log_debug("HttpContext Body");
            request_.setBody(buf->retrieveAllAsString());
            hasMore = false;
        }
    }

    log_debug("HttpContext ParseRequest end");
    return ok;
}

bool HttpContext::processRequestLine(const char *start, const char *end)
{
    log_debug("HttpContext::processRequestLint running");
    bool succeed = false;
    const char *begin = start;
    const char *space = std::find(start, end, ' ');
    // POST /test HTTP/1.1   or   GET /test?username=abc&password=456 HTTP/1.1
    if (space != end && request_.setMethod(start, space))
    {
        begin = space + 1;
        space = std::find(begin, end, ' ');
        // GET /test?username=abc&password=456 HTTP/1.1
        if (space != end)
        {
            const char *question = std::find(begin, space, '?');
            if (question != space)
            {
                request_.setPath(begin, question);
                request_.setQuery(question, space);
            }
            else
            {
                request_.setPath(begin, space);
            }

            begin = space + 1;
            succeed = end - begin == 8 && std::equal(begin, end - 1, "HTTP/1.");
            if (succeed)
            {
                if (*(end - 1) == '1')
                {
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else if (*(end - 1) == '0')
                {
                    request_.setVersion(HttpRequest::kHttp10);
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}