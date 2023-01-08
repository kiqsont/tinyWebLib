#include "LoggerUtil.h"

#include "ProcessInfo.h"
#include <cstdio>
#include <cassert>
#include <cstring>

using namespace asyncLogger::detail;

thread_local char t_errnobuf[512]{0};
thread_local char t_time[64];
thread_local tm t_tm;
thread_local time_t t_lastSecond;

// isTime is for if return string with "hour:min:second"
const char *Util::getCurDateTime(bool isTime, time_t *now)
{
    time_t timer = time(nullptr);
    if (nullptr != now)
    {
        *now = timer;
    }
    if (t_lastSecond != timer)
    {
        t_lastSecond = timer;
        localtime_r(&timer, &t_tm);
    }
    int len;
    if (isTime)
    {
        len = snprintf(t_time, sizeof t_time, "%4d-%02d-%02d %02d:%02d:%02d",
                       t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday,
                       t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
        assert(len == 19);
    }
    else
    {
        len = snprintf(t_time, sizeof t_time, "%4d-%02d-%02d",
                       t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday);
        assert(len == 10);
    }
    return t_time;
}
const char *Util::getErrorInfo(int error_code)
{
    return strerror_r(error_code, t_errnobuf, sizeof t_errnobuf);
}
std::string Util::getLogFileName(const std::string &basename, time_t &now)
{
    std::string filename;

    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32]{0};
    tm tempTm{};
    localtime_r(&now, &tempTm);
    strftime(timebuf, sizeof timebuf, ".%Y.%m.%d-%H.%M.%S.", &tempTm);
    filename += timebuf;

    char pidbuf[32]{0};
    snprintf(pidbuf, sizeof pidbuf, ".%d", asyncLogger::ProcessInfo::GetPid());
    filename += pidbuf;
    return filename;
}