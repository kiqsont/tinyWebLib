#include "TimingWheel.h"
// #define LOG_DEBUG
#include "Logger.h"

using namespace asyncLogger;

namespace timingWheel
{
    void connectionTimingWheel(const TcpConnectionPtr &conn, std::shared_ptr<WeakConnectionList> &connectionBuckets)
    {
        if (conn->connected())
        {
            EntryPtr entry(std::make_shared<Entry>(conn));
            connectionBuckets->back().insert(entry);
            WeakEntryPtr weakEntry(entry);
            conn->setContext(weakEntry);
            log_debug("connectionTimingWheel insert Entry:{}", entry.use_count());
        }
        else
        {
            WeakEntryPtr weakEntryPtr(std::any_cast<WeakEntryPtr>(conn->getContext()));
            log_debug("ConnectionCallback for timing wheel disconnection: Entry use_count={}", weakEntryPtr.use_count());
        }
    }

    void messageTimingWheel(const TcpConnectionPtr &conn, std::shared_ptr<WeakConnectionList> &connectionBuckets)
    {
        WeakEntryPtr weakEntry(std::any_cast<WeakEntryPtr>(conn->getContext()));
        EntryPtr entry(weakEntry.lock());
        if (entry)
            connectionBuckets->back().insert(entry);
        log_debug("TimingWheel::messageCallback");
    }
}