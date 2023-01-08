#include "Config.h"

using namespace asyncLogger::asyncLoggerDetail;

Config &Config::instance()
{
    static Config configInstance;
    return configInstance;
}
