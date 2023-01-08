#ifndef ASYNC_LOGGER_LOGGERUTIL
#define ASYNC_LOGGER_LOGGERUTIL

#include <ctime>
#include <string>

namespace asyncLogger::asyncLoggerDetail
{
    struct Util
    {
        static const char *getCurDateTime(bool isTime, time_t *now = nullptr);
        static const char *getErrorInfo(int error_code);
        static std::string getLogFileName(const std::string &basename, time_t &now);
    };
}

#endif // ASYNC_LOGGER_LOGGERUTIL