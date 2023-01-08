#include "TimingWheel.h"

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
        }
        else
        {
            WeakEntryPtr weakEntryPtr(std::any_cast<WeakEntryPtr>(conn->getContext()));
            LOG_DEBUG("ConnectionCallback for timing wheel disconnection: Entry use_count=%d", )
        }
    }

    void messageTimingWheel(const TcpConnectionPtr &conn, std::shared_ptr<WeakConnectionList> &connectionBuckets)
    {
        WeakEntryPtr weakEntry(std::any_cast<WeakEntryPtr>(conn->getContext()));
        EntryPtr entry(weakEntry.lock());
        if (entry)
            connectionBuckets->back().insert(entry);
    }
}