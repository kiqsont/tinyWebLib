#ifndef KIQSONT_MUDUO_COPY_THREAD
#define KIQSONT_MUDUO_COPY_THREAD

#include "noncopyable.h"

#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include <string>

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

public:
    explicit Thread(ThreadFunc func, const std::string &name = std::string());

    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    const std::string &name() const { return name_; }
    unsigned long getThreadId() const;

    static int numCreated() { return numCreated_; }

private:
    bool started_ = false;
    bool joined_ = false;
    std::thread::id id_;
    std::shared_ptr<std::thread> thread_;
    ThreadFunc func_;
    std::string name_{""};
    inline static std::atomic_int32_t numCreated_{0};
};

#endif // KIQSONT_MUDUO_COPY_THREAD
