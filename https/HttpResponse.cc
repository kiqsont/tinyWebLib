#include "HttpResponse.h"

#include <cstdio>
#include <cstring>

void HttpResponse::appendToBuffer(Buffer *output)
{
    std::string responseLine = "HTTP/1.1 " + std::to_string(statusCode_) + " ";
    output->append(responseLine);
    output->append(statusMessage_);
    output->append("\r\n");

    if (closeConnection_)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        std::string bodyLength = "Content-Length: " + std::to_string(body_.size());
        output->append(bodyLength);
        output->append("\r\n");
        std::string keepAlive = "Connection: Keep-Alive\r\n";
        output->append(keepAlive.c_str(), keepAlive.size());
    }

    for (const auto &header : headers_)
    {
        std::string temp = ": ";
        output->append(header.first.c_str(), header.first.size());
        output->append(temp.c_str(), temp.size());
        output->append(header.second.c_str(), header.second.size());
        output->append("\r\n", 2);
    }

    output->append("\r\n", 2);
    output->append(body_.c_str(), body_.size());
}