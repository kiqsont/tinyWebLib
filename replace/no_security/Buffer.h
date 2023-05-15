#ifndef KIQSONT_MUDUO_OVERWRITE_BUFFER
#define KIQSONT_MUDUO_OVERWRITE_BUFFER

#include "noncopyable.h"

#include <vector>
#include <cstring>
#include <string>
#include <algorithm>
#include <assert.h>

class Buffer : noncopyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    inline static const char tempCRLF[] = "\r\n";

public:
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend)
    {
    }
    Buffer(Buffer &&) = default;
    Buffer &operator=(Buffer &&) = default;

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writeableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    char *prependPeek()
    {
        return begin();
    }

    // the beginning address for readable buffer
    const char *peek() const
    {
        return begin() + readerIndex_;
    }

    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else // len == readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void ensureWriteableBytes(size_t len)
    {
        if (writeableBytes() < len)
        {
            makeSpace(len);
        }
    }

    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }
    void append(const char *str)
    {
        append(str, strlen(str));
    }

    void append(const std::string &str)
    {
        append(str.c_str(), str.size());
    }

    char *beginWrite()
    {
        return begin() + writerIndex_;
    }
    const char *beginWrite() const
    {
        return begin() + writerIndex_;
    }

    // read from fd
    ssize_t readFd(int fd, int *saveErrno);
    ssize_t writeFd(int fd, int *saveErrno);

    const char *findCRLF(const char *start = nullptr) const
    {
        if (start != nullptr)
        {
            assert(peek() <= start);
            assert(start <= beginWrite());
        }
        else
        {
            start = peek();
        }
        const char *crlf = std::search(start, beginWrite(), tempCRLF, tempCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

private:
    char *begin()
    {
        return buffer_.data();
    }
    const char *begin() const
    {
        return buffer_.data();
    }

    void makeSpace(size_t len)
    {
        if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

#endif // KIQSONT_MUDUO_OVERWRITE_BUFFER
