#ifndef KIQSONT_MUDUO_OVERWRITE_TIMINGWHEEL
#define KIQSONT_MUDUO_OVERWRITE_TIMINGWHEEL

#include <list>
#include <unordered_set>
#include "Callbacks.h"
#include "TcpConnection.h"
#include "TimerID.h"

// #define LOG_DEBUG
#include "Logger.h"
using namespace asyncLogger;

namespace timingWheel
{
    using WeakTcpConnectionPtr = std::weak_ptr<TcpConnection>;
    struct Entry
    {
        explicit Entry(const WeakTcpConnectionPtr &weakConn)
            : weakConn_(weakConn)
        {
        }

        ~Entry()
        {
            TcpConnectionPtr conn = weakConn_.lock();
            if (conn)
            {
                log_debug("conn->shutdown by weakptr");
                // conn->shutdown();
                conn->foreClose();
            }
        }

        WeakTcpConnectionPtr weakConn_;
    };

    using EntryPtr = std::shared_ptr<Entry>;
    using WeakEntryPtr = std::weak_ptr<Entry>;
    using Bucket = std::unordered_set<EntryPtr>;

    struct WeakConnectionList
    {
        explicit WeakConnectionList(int maxSize) : maxSize_(maxSize), queue_(maxSize),timerID_(Timestamp::invalid()) {}

        int getMaxSize() const
        {
            return maxSize_;
        }

        void push_back(Bucket bucket)
        {
            queue_.emplace_back(std::move(bucket));
            queue_.pop_front();
        }

        Bucket &back()
        {
            return queue_.back();
        }

        int maxSize_;
        std::list<Bucket> queue_;
        TimerID timerID_;
    };

    void connectionTimingWheel(const TcpConnectionPtr &conn, std::shared_ptr<WeakConnectionList> &connectionBuckets);
    void messageTimingWheel(const TcpConnectionPtr &conn, std::shared_ptr<WeakConnectionList> &connectionBuckets);
}

#endif // KIQSONT_MUDUO_OVERWRITE_TIMINGWHEEL