#include "FileAppender.h"

#include "LoggerUtil.h"
#include "Logger.h"
#include <unistd.h>

using namespace asyncLogger::asyncLoggerDetail;

FileAppender::FileAppender(const std::string &filename)
{
    init(filename);
}
void FileAppender::init(const std::string &filename)
{
    std::string fileDir;
    size_t pos;
    if ((pos = filename.rfind('/')) == std::string::npos)
    {
        trace("Invalid Dir Path{}", filename);
        throw std::runtime_error("invalid filepath");
    }

    fileDir = filename.substr(0, pos + 1);
    if (access(fileDir.c_str(), F_OK) == -1)
    {
        trace("File Path doesn't Exist");
        throw std::runtime_error(fmt::format("file dir not exist!{}", fileDir));
    }

    m_file = fopen(filename.c_str(), "ae");
    if (nullptr == m_file)
    {
        int err = ferror(m_file);
        auto *errorInfo = Util::getErrorInfo(err);
        trace("FileAppender init Failed for err:{}", errorInfo);
        fprintf(stderr, "FileAppender error in open file:%s errno:%s", filename.c_str(), errorInfo);
        return;
    }
}

FileAppender::~FileAppender() noexcept
{
    if (nullptr != m_file)
    {
        ::fflush(m_file);
        ::fclose(m_file);
    }
}

void FileAppender::append(const char *message, size_t len)
{
    size_t written = 0;
    while (written != len)
    {
        size_t remain = len - written;
        size_t n = write(message + written, remain);
        if (n != remain)
        {
            int err = ferror(m_file);
            if (err)
            {
                trace("FileAppender::append tried to write data but failed,err:{}", Util::getErrorInfo(err));
                fprintf(stderr, "AppendFile::append() failed to write err:%s", Util::getErrorInfo(err));
                break;
            }
            if (n == 0)
            {
                throw std::runtime_error("write failed,FILE is empty");
            }
        }
        written += n;
    }
    m_writenBytes += written;
}

void FileAppender::flush()
{
    if (m_file)
    {
        trace("FileAppender::flush used ::fflush");
        ::fflush(m_file);
    }
}

size_t FileAppender::write(const char *message, size_t len)
{
    size_t sz = 0;
    if (m_file)
    {
        trace("FileAppender::write fwrite");
        sz = fwrite_unlocked(message, 1, len, m_file);
    }
    return sz;
}
