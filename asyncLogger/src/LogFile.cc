#include "LogFile.h"

#include "LoggerUtil.h"
#include "Config.h"
#include "Logger.h"
#include <iostream>

using namespace asyncLogger::detail;

LogFile::LogFile(const std::string &basename, off64_t rollSize, bool threadSafe, int flushInterval, int checkEveryN)
    : m_basename(basename), m_rollSize(rollSize), m_flushInterval(flushInterval), m_checkEveryN(checkEveryN), m_mutex(threadSafe ? std::make_unique<std::mutex>() : nullptr)
{
    rollFile();
}

void LogFile::append(const char *message, size_t len)
{
    if (m_mutex)
    {
        std::lock_guard<std::mutex> lock(*m_mutex);
        append_unlocked(message, len);
    }
    else
    {
        append_unlocked(message, len);
    }
}

void LogFile::flush()
{
    if (m_mutex)
    {
        std::lock_guard<std::mutex> lock(*m_mutex);
        m_file->flush();
    }
    else
    {
        m_file->flush();
    }
}

// creat a new log file to write data
void LogFile::rollFile(const time_t *now)
{
    time_t temp_now;
    if (now != nullptr)
        temp_now = *now;
    else
        temp_now = time(nullptr);

    auto filename = Util::getLogFileName(m_basename, temp_now);
    auto start = temp_now / kRollPerSeconds * kRollPerSeconds;

    if (temp_now > m_lastRoll)
    {
        trace("Begin to roll to filename:{}", filename);

        m_lastRoll = temp_now;
        m_lastFlush = temp_now;
        m_lastPeriod = start;
        m_file = std::make_unique<FileAppender>(filename);
    }
}

void LogFile::append_unlocked(const char *message, size_t len)
{
    m_file->append(message, len);
    if (m_file->writtenBytes() > m_rollSize)
    {
        trace("Log file hasn't space to write{:d},begin to build a new log file", m_rollSize / 1024);
        rollFile();
        m_file->resetWritten();
    }
    else
    {
        ++m_count;
        if (m_count >= m_checkEveryN)
        {
            trace("Begin to check flushInterval and curPeriod");
            m_count = 0;
            time_t now = ::time(nullptr);
            time_t curPeriod = now / kRollPerSeconds * kRollPerSeconds;
            if (curPeriod != m_lastPeriod)
            {
                rollFile(&now);
            }
            else if (now - m_lastFlush > m_flushInterval)
            {
                m_lastFlush = now;
                m_file->flush();
            }
        }
    }
}
