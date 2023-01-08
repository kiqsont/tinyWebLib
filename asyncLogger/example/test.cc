#include "Logger.h"
#include "../dependencies/Timer.hpp"

using namespace asyncLogger;

int main()
{
    Config::Set({

        .output_prefix = "pre info",
        .output_basedir = "./log/logFile",
        .is_console = false,

    });
    Timer timer;
    for (int i = 0; i < 1000000; i++)
    {
        info("log info");
    }
    timer.Stop();
}