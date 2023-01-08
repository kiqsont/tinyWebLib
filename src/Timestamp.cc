#include "Timestamp.h"

#include <time.h>

Timestamp::Timestamp() : microSecondsSinceEpoch_(time(NULL) * (int64_t)kMicroSecondsPerSecond) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL) * (int64_t)kMicroSecondsPerSecond);
}

/*
std::string Timestamp::toString() const
{
    char buf[128]{0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}
*/
std::string Timestamp::toString() const
{
    char buf[128]{0};
    int64_t tempTime = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    tm *tm_time = localtime(&tempTime);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}

Timestamp Timestamp::afterNow(double seconds)
{
    return Timestamp::now().addTime(seconds);
}

Timestamp Timestamp::addTime(double seconds) const
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(microSecondsSinceEpoch_ + delta);
}
