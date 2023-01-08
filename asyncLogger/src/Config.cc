#include "Config.h"

using namespace asyncLogger::detail;

Config &Config::instance()
{
    static Config configInstance;
    return configInstance;
}
