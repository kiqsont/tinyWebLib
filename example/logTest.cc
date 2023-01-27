#define LOG_TRACE
#define LOG_DEBUG
#include "Logger.h"
#include "../asyncLogger/dependencies/Timer.hpp"

using namespace asyncLogger;

void logFile()
{
    // 如果想全局开启log_trace功能，去asyncLogger/include/Logger.h 把LOG_TRACE宏开启
    // log_trace不会输出到文件
    Config::Set({
        .output_prefix = "prefix tag for file",
        .output_basedir = "../log/logFileTest",
        .is_console = false,
    });

    log_debug("log debug");
    log_info("log info");
    log_warn("log warn");
    log_error("log error");
    log_trace("log trace");

    Timer timer;
    for (int i = 0; i < 10000; i++)
    {
        log_info("log --- info");
    }
    timer.Stop();
}

void logConsole()
{
    Config::Set({
        .output_prefix = "prefix tag for console",
        .is_console = true,
    });

    log_debug("log debug to console");
    log_info("log info to console");
    log_warn("log warn to console");
    log_error("log error to console");
    log_trace("log trace to console");
}

int main()
{
    // 使用log功能需要链接asyncLogger库，并且设置好对应的inclue
    // 这两个测试不能同时运行，因为他们的output_basedir不一致，而loglib又是单例的
    // logFile();
    logConsole();
}