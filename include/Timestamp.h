#ifndef KIQSONT_MUDUO_OVERWRITE_TIMESTAMP
#define KIQSONT_MUDUO_OVERWRITE_TIMESTAMP

#include <iostream>
#include <string>
#include <algorithm>

/**
 * 这里没有用chrono库，因为gcc对C++20的format不支持，导致time_point不好处理
 */

class Timestamp
{
public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

public:
    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();

    std::string toString() const;

    Timestamp addTime(double seconds) const;

    bool valid() const
    {
        return microSecondsSinceEpoch_ > 0;
    }

    static Timestamp invalid()
    {
        return Timestamp(0);
    }

    static Timestamp afterNow(double seconds);

    int64_t getRawTime() const { return microSecondsSinceEpoch_; }

private:
    int64_t microSecondsSinceEpoch_ = 0;
};

inline bool operator==(const Timestamp &lhs, const Timestamp &rhs)
{
    return lhs.getRawTime() == rhs.getRawTime();
}

inline bool operator<(const Timestamp &lhs, const Timestamp &rhs)
{
    return lhs.getRawTime() < rhs.getRawTime();
}

#endif // KIQSONT_MUDUO_OVERWRITE_TIMESTAMP
