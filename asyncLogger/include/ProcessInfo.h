#ifndef ASYNC_LOGGER_PROCESSINFO
#define ASYNC_LOGGER_PROCESSINFO

#include <sys/types.h>
#include <unistd.h>
#include <string>

namespace asyncLogger
{
    struct ProcessInfo
    {
        static pid_t GetPid();
        static pid_t GetTid();
        static pid_t GetUid();
        static std::string GetHostname();
    };

}

#endif // ASYNC_LOGGER_PROCESSINFO