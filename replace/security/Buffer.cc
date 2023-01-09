#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <openssl/err.h>

// works in LT
ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536]{0}; // 64k space from stack
    struct iovec vec[2]{0};
    const size_t writable = writeableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // buffer could handle datas
    {
        writerIndex_ += n;
    }
    else // extrabuf has datas
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}

int Buffer::readSSL(SSL *ssl, int *saveErrno)
{
    ensureWriteableBytes(1024);
    int n = ::SSL_read(ssl, begin() + writerIndex_, writeableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else
    {
        writerIndex_ += n;
    }
    return n;
}

int Buffer::writeSSL(SSL *ssl, int *saveErrno)
{
    int n = ::SSL_write(ssl, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}