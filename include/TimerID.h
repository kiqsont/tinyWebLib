#ifndef KIQSONT_MUDUO_OVERWRITE_TIMERID
#define KIQSONT_MUDUO_OVERWRITE_TIMERID

#include "Timestamp.h"
#include <atomic>

/**
 * user can use TimerID to cancel the timer by sequence
 */
class TimerID
{
public:
    TimerID(Timestamp when) : expiration_(when), sequence_(++s_numCreated_) {}
    TimerID(Timestamp when, uint64_t sequence) : expiration_(when), sequence_(sequence) {}

    Timestamp getExpirationTime() const
    {
        return expiration_;
    }

    uint64_t getSequence() const
    {
        return sequence_;
    }

protected:
    Timestamp expiration_;
    uint64_t sequence_ = 0;

    inline static std::atomic_uint64_t s_numCreated_{0};

    friend class CompableTimer;
};

#endif // KIQSONT_MUDUO_OVERWRITE_TIMERID