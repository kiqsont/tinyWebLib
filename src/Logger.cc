#include "Logger.h"
#include "Timestamp.h"

#include <iostream>

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int Level)
{
    logLevel_ = Level;
}

// write log
void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    case WARNING:
        std::cout << "[WARNING]";
        break;
    case TRACE:
        std::cout << "[TRACE]";
        break;
    }

    // print time and msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}
