#ifndef KIQSONT_MUDUO_COPY_LOGGER
#define KIQSONT_MUDUO_COPY_LOGGER

//#define MUDUO_LOGINFO
//#define MUDUO_DEBUG

#include <string>
#include <unistd.h>

#include "noncopyable.h"

// LOG_INFO("%s %d", arg1, arg2)
// can't use like: LOG_INFO("%s",buf),and buf is a char[] which like char buf[64]
#ifdef MUDUO_LOGINFO
#define LOG_INFO(logmsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(INFO);                         \
        char buf[1024]{0};                                \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)
#else
#define LOG_INFO(logmsgFormat, ...)
#endif

#define LOG_ERROR(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(ERROR);                        \
        char buf[1024]{0};                                \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_FATAL(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(FATAL);                        \
        char buf[1024]{0};                                \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)

#ifdef MUDUO_DEBUG
#define LOG_DEBUG(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(DEBUG);                        \
        char buf[1024]{0};                                \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        sleep(1);                                         \
    } while (0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif

#define LOG_WARNING(logmsgFormat, ...)                    \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(WARNING);                      \
        char buf[1024]{0};                                \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)

#define LOG_TRACE(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(TRACE);                        \
        char buf[1024]{0};                                \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)

enum LogLevel
{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
    WARNING,
    TRACE
};

class Logger : noncopyable
{
public:
    static Logger &instance();

    void setLogLevel(int Level);

    void log(std::string msg);

private:
    int logLevel_;
    Logger() {}
};

#endif // KIQSONT_MUDUO_COPY_LOGGER
