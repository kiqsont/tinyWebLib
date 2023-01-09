
#ifndef MUDUO_COPY_HTTP_REQUEST
#define MUDUO_COPY_HTTP_REQUEST

#include <muduo_copy/Timestamp.h>
#include <map>
#include <string>
#include <assert.h>
#include <memory>
#include <cstdio>

class TcpConnection;

class HttpRequest
{
public:
    enum Method
    {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete
    };
    enum Version
    {
        kUnkonw,
        kHttp10,
        kHttp11
    };

public:
    void setVersion(Version v)
    {
        version_ = v;
    }
    Version getVersion() const
    {
        return version_;
    }

    bool setMethod(const char *start, const char *end)
    {
        assert(method_ == kInvalid);
        std::string method(start, end);
        if ("GET" == method)
        {
            method_ = kGet;
        }
        else if ("POST" == method)
        {
            method_ = kPost;
        }
        else if ("HEAD" == method)
        {
            method_ = kHead;
        }
        else if ("PUT" == method)
        {
            method_ = kPut;
        }
        else if ("DELETE" == method)
        {
            method_ = kDelete;
        }
        else
        {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }
    Method method() const
    {
        return method_;
    }
    const char *methodString() const
    {
        switch (method_)
        {
        case kGet:
            return "GET";
        case kPost:
            return "POST";
        case kHead:
            return "HEAD";
        case kPut:
            return "PUT";
        case kDelete:
            return "DELETE";
        default:
            return "UNKNOW";
        }
    }

    const char *versionToString() const
    {
        switch (version_)
        {
        case kHttp10:
            return "HTTP/1.0";
        case kHttp11:
            return "HTTP/1.1";
        default:
            return "unKnown";
        }
    }

    void setPath(const char *start, const char *end)
    {
        path_.assign(start, end);
    }
    const std::string &path() const
    {
        return path_;
    }

    void setQuery(const char *start, const char *end)
    {
        query_.assign(start, end);
    }
    const std::string &query() const
    {
        return query_;
    }
    void setBody(const std::string &body)
    {
        body_ = body;
    }
    const std::string &body() const
    {
        return body_;
    }

    void setReceiveTime(Timestamp t)
    {
        receiveTime_ = t;
    }
    Timestamp receiveTime() const
    {
        return receiveTime_;
    }

    void addHeader(const char *start, const char *colon, const char *end)
    {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }

        std::string value(colon, end);
        while (!value.empty() && isspace(*(value.end() - 1)))
        {
            value.resize(value.size() - 1);
        }
        headers_[field] = std::move(value);
    }
    std::string getHeader(const std::string &field) const
    {
        std::string result;
        auto it = headers_.find(field);
        if (it != headers_.end())
        {
            result = it->second;
        }
        return result;
    }

    const std::map<std::string, std::string> &headers() const
    {
        return headers_;
    }

    void swap(HttpRequest &rhs)
    {
        std::swap(method_, rhs.method_);
        std::swap(version_, rhs.version_);
        path_.swap(rhs.path_);
        query_.swap(rhs.query_);
        std::swap(receiveTime_, rhs.receiveTime_);
        headers_.swap(rhs.headers_);
    }

    void setConnection(const std::shared_ptr<TcpConnection> &conn)
    {
        conn_ = conn;
    }
    std::shared_ptr<TcpConnection> getConnection() const
    {
        return conn_;
    }

private:
    Method method_ = kInvalid;
    Version version_ = kUnkonw;
    std::string path_;
    std::string query_;
    std::string body_;
    Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
    std::shared_ptr<TcpConnection> conn_;
};

#endif // MUDUO_COPY_HTTP_REQUEST