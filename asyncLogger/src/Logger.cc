#include "Logger.h"

#include <errno.h>
#include <cstring>

#include "fmt/chrono.h"
#include "LoggerUtil.h"
#include "ProcessInfo.h"

using namespace asyncLogger;
using namespace asyncLogger::detail;

#define LOG_CONFIG detail::Config::instance()

void asyncLogger::Config::Set(const asyncLogger::Config &config)
{
    LOG_CONFIG.basename() = config.output_basedir;
    LOG_CONFIG.flags() = config.print_flag;
    LOG_CONFIG.console() = config.is_console;
    LOG_CONFIG.prefix() = config.output_prefix;
    LOG_CONFIG.before() = config.before;
    LOG_CONFIG.end() = config.end;
}

const char *s_level_text[LEVEL_COUNT + 1] = {"[DEBUG]  ", "[INFO]   ",
                                             "[WARN]   ", "[ERROR]  ",
                                             "[FATAL]  ", "[UNKNOWN]"};

fmt::color s_color[LEVEL_COUNT + 1] = {
    fmt::color::blue,
    fmt::color::green,
    fmt::color::yellow,
    fmt::color::red,
    fmt::color::purple,
    fmt::color::orange_red,
};

inline const char *GET_LEVEL_TEXT(int level)
{
    if (level >= LEVEL_COUNT || level < 0)
        return s_level_text[LEVEL_COUNT];
    return s_level_text[level];
}

inline fmt::color GET_COLOR_BY_LEVEL(int level)
{
    if (level >= LEVEL_COUNT || level < 0)
        return s_color[LEVEL_COUNT];
    return s_color[level];
}

detail::Logger::Logger() { init_data(); }

detail::Logger &detail::Logger::getInstance()
{
    static Logger logger;
    return logger;
}

void detail::Logger::init_data()
{
    if (LOG_CONFIG.basename() != nullptr)
    {
        m_logging =
            std::make_unique<AsyncLogging>(LOG_CONFIG.basename(), LOG_CONFIG.rollSize(), LOG_CONFIG.flushInterval());
    }
}

void detail::Logger::interval_log(const context &ctx)
{
    flockfile(stdout);
    fmt::print(stdout, fg(fmt::color::green),
               "[LOG] [pid:{}] [tid:{}] {:%Y-%m-%d -%H:%M:%S} {}:{:d} {}\r\n", ProcessInfo::GetPid(), ProcessInfo::GetTid(), fmt::localtime(time(nullptr)), ctx.short_filename, ctx.line, ctx.text);
    funlockfile(stdout);
}

void detail::Logger::LogFile(detail::context const &ctx)
{
    int tid = ProcessInfo::GetTid();
    auto *level_text = GET_LEVEL_TEXT(ctx.level);
    const char *filename = LOG_CONFIG.flags() & Llongname ? ctx.long_filename : ((LOG_CONFIG.flags() & Lshortname) ? ctx.short_filename : nullptr);

    fmt::memory_buffer buffer;

    // date
    if (LOG_CONFIG.flags() & Ldata)
    {
        fmt::format_to(std::back_inserter(buffer), "{} ",
                       Util::getCurDateTime(LOG_CONFIG.flags() & Ltime));
    }

    // prefix
    if (LOG_CONFIG.prefix())
    {
        fmt::format_to(std::back_inserter(buffer), "{} ", LOG_CONFIG.prefix());
    }

    // before callback
    if (LOG_CONFIG.before())
    {
        LOG_CONFIG.before()(buffer);
    }

    // level
    fmt::format_to(std::back_inserter(buffer), "{}", level_text);

    // thread ID
    if (LOG_CONFIG.flags() & LthreadId)
    {
        fmt::format_to(std::back_inserter(buffer), " [tid:{:d}] ", tid);
    }

    // filename and line
    if (filename != nullptr)
    {
        fmt::format_to(std::back_inserter(buffer), " [{}:{:d}] ", filename,
                       ctx.line);
    }

    // end callback
    if (LOG_CONFIG.end())
    {
        LOG_CONFIG.end()(buffer);
    }

    // FATAL!!!
    if (ctx.level == L_FATAL)
    {
        fmt::format_to(std::back_inserter(buffer), " {}:{} \r\n", ctx.text,
                       Util::getErrorInfo(errno));
    }
    else
    {
        fmt::format_to(std::back_inserter(buffer), " {} \r\n",
                       ctx.text);
    }

    // write data to buffer
    m_logging->append(buffer.data(), buffer.size());
}

void detail::Logger::LogConsole(const asyncLogger::detail::context &ctx)
{
    flockfile(stdout);

    int tid = ProcessInfo::GetTid();
    auto *level_text = GET_LEVEL_TEXT(ctx.level);
    const char *filename = LOG_CONFIG.flags() & Llongname ? ctx.long_filename : ((LOG_CONFIG.flags() & Lshortname) ? ctx.short_filename : nullptr);

    fmt::memory_buffer buffer;

    // date
    if (LOG_CONFIG.flags() & Ldata)
    {
        fmt::format_to(std::back_inserter(buffer), "{} ",
                       Util::getCurDateTime(LOG_CONFIG.flags() & Ltime));
    }

    // prefix
    if (LOG_CONFIG.prefix())
    {
        fmt::format_to(std::back_inserter(buffer), "{} ", LOG_CONFIG.prefix());
    }

    // before callback
    if (LOG_CONFIG.before())
    {
        LOG_CONFIG.before()(buffer);
    }

    // level
    fmt::format_to(std::back_inserter(buffer), "{}", level_text);

    // thread ID
    if (LOG_CONFIG.flags() & LthreadId)
    {
        fmt::format_to(std::back_inserter(buffer), " [tid:{:d}] ", tid);
    }

    // filename and line
    if (filename != nullptr)
    {
        fmt::format_to(std::back_inserter(buffer), " [{}:{:d}] ", filename,
                       ctx.line);
    }

    // end callback
    if (LOG_CONFIG.end())
    {
        LOG_CONFIG.end()(buffer);
    }

    // FATAL!!!
    if (ctx.level == L_FATAL || ctx.level == L_ERROR && errno)
    {
        fmt::format_to(std::back_inserter(buffer), fg(GET_COLOR_BY_LEVEL(ctx.level)), " {}:{} \r\n", ctx.text,
                       Util::getErrorInfo(errno));
    }
    else
    {
        fmt::format_to(std::back_inserter(buffer),
                       fg(GET_COLOR_BY_LEVEL(ctx.level)),
                       " {} \r\n",
                       ctx.text);
    }

    fmt::print(stdout, fmt::runtime(to_string(buffer)));

    fflush(stdout);
    funlockfile(stdout);
}

void detail::Logger::DoLog(detail::context const &ctx)
{
    if (m_logging)
    {
        LogFile(ctx);
    }

    if (LOG_CONFIG.console())
    {
        LogConsole(ctx);
    }
    if (ctx.level == L_FATAL)
    {
        abort();
    }
}