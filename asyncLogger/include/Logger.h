#ifndef ASYNC_LOGGER_LOGGER
#define ASYNC_LOGGER_LOGGER

// for log trace
// #define LOG_TRACE

// for Release
#ifndef LOG_DEBUG
#define NDEBUG
#endif

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "fmt/color.h"

#include "noncopyableLog.h"
#include "Config.h"
#include "AsyncLogging.h"

namespace asyncLogger
{
// for trace
#ifdef LOG_TRACE
#define TRACE_
#endif

// for debug
#ifndef NDEBUG
#define DEBUG_
#define INFO_
#define WARN_
#define ERROR_
#define FATAL_
#endif

// for release
#ifdef NDEBUG
#undef DEBUG_
#define INFO_
#define WARN_
#define ERROR_
#define FATAL_
#endif

// for just warnning
#ifdef LOG_LIMIT_WARN
#undef DEBUG_
#undef INFO_
#define WARN_
#define ERROR_
#define FATAL_
#endif

// for just error
#ifdef LOG_LIMIT_ERROR
#undef DEBUG_
#undef INFO_
#undef WARN_
#define ERROR_
#define FATAL_
#endif

    using std::string;

    struct Config
    {
        int print_flag = LstdFlags;
        const char *output_prefix = nullptr;
        const char *output_basedir = nullptr;
        bool is_console = true;
        asyncLoggerDetail::callback_t before;
        asyncLoggerDetail::callback_t end;

        static void Set(const Config &config);
    };

    namespace asyncLoggerDetail
    {
        struct context;

        class Logger : noncopyableLog
        {
        private:
            Logger();
            void init_data();

        public:
            static Logger &getInstance();
            void DoLog(context const &ctx);
            void LogFile(context const &ctx);
            static void LogConsole(context const &ctx);
            static void interval_log(context const &ctx);

        private:
            std::unique_ptr<AsyncLogging> m_logging;
        };

        struct context
        {
            int level;
            int line;
            const char *short_filename{};
            const char *long_filename{};
            string text;
        };

        inline void InternalLog(context &ctx)
        {
            asyncLoggerDetail::Logger::interval_log(ctx);
        }

        inline void DoLog(context &ctx)
        {
            asyncLoggerDetail::Logger::getInstance().DoLog(ctx);
        }

        inline const char *getShortName(const char *filename)
        {
            int len = strlen(filename);
            int pos;
            for (int i = len - 1; i >= 0; i--)
            {
                if (filename[i] == '/' || filename[i] == '\\')
                {
                    pos = i + 1;
                    break;
                }
            }
            return filename + pos;
        }
    }

#define ASYNC_NAMESPACE asyncLogger::asyncLoggerDetail::

#define INIT_LOG_(level_)         \
    ctx.level = level_;           \
    ctx.line = __LINE__;          \
    ctx.long_filename = __FILE__; \
    ctx.short_filename = ASYNC_NAMESPACE getShortName(__FILE__);

#ifdef TRACE_
#define log_trace(fmt_, args_...)              \
    do                                         \
    {                                          \
        ASYNC_NAMESPACE context ctx;           \
        ctx.text = fmt::format(fmt_, ##args_); \
        INIT_LOG_(0)                           \
        asyncLoggerDetail::InternalLog(ctx);   \
    } while (false)
#else
#define log_trace(format, args...)
#endif

#ifdef DEBUG_
#define log_debug(fmt_, args_...)              \
    do                                         \
    {                                          \
        ASYNC_NAMESPACE context ctx;           \
        ctx.text = fmt::format(fmt_, ##args_); \
        INIT_LOG_(0)                           \
        asyncLoggerDetail::DoLog(ctx);         \
    } while (false)
#else
#define log_debug(format, args...)
#endif

#ifdef INFO_
#define log_info(fmt_, args_...)               \
    do                                         \
    {                                          \
        ASYNC_NAMESPACE context ctx;           \
        ctx.text = fmt::format(fmt_, ##args_); \
        INIT_LOG_(1)                           \
        asyncLoggerDetail::DoLog(ctx);         \
    } while (false)
#else
#define log_info(format, args...)
#endif

#ifdef WARN_
#define log_warn(fmt_, args_...)               \
    do                                         \
    {                                          \
        ASYNC_NAMESPACE context ctx;           \
        ctx.text = fmt::format(fmt_, ##args_); \
        INIT_LOG_(2)                           \
        asyncLoggerDetail::DoLog(ctx);         \
    } while (false)
#else
#define log_warn(format, args...)
#endif

#ifdef ERROR_
#define log_error(fmt_, args_...)              \
    do                                         \
    {                                          \
        ASYNC_NAMESPACE context ctx;           \
        ctx.text = fmt::format(fmt_, ##args_); \
        INIT_LOG_(3)                           \
        asyncLoggerDetail::DoLog(ctx);         \
    } while (false)
#else
#define log_error(format, args...)
#endif

#ifdef FATAL_
#define log_fatal(fmt_, args_...)              \
    do                                         \
    {                                          \
        ASYNC_NAMESPACE context ctx;           \
        ctx.text = fmt::format(fmt_, ##args_); \
        INIT_LOG_(4)                           \
        asyncLoggerDetail::DoLog(ctx);         \
    } while (false)
#else
#define log_fatal(format, args...)
#endif
}
#endif // ASYNC_LOGGER_LOGGER