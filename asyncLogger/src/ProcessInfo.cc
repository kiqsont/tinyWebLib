#include "ProcessInfo.h"

#include <syscall.h>

using namespace asyncLogger;
pid_t ProcessInfo::GetPid()
{
    thread_local pid_t pid = getpid();
    return pid;
}
pid_t ProcessInfo::GetTid()
{
    thread_local auto tid = static_cast<pid_t>(syscall(SYS_gettid));
    return tid;
}
pid_t ProcessInfo::GetUid()
{
    thread_local uid_t uid = getuid();
    return uid;
}
std::string ProcessInfo::GetHostname()
{
    char buf[256]{0};
    if (gethostname(buf, sizeof buf) == 0)
    {
        buf[sizeof buf - 1] = 0;
        return buf;
    }
    return "unknownhost";
}
