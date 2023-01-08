#ifndef ASYNC_LOGGER_noncopyableLog
#define ASYNC_LOGGER_noncopyableLog
class noncopyableLog
{
public:
    noncopyableLog(const noncopyableLog &) = delete;
    noncopyableLog &operator=(const noncopyableLog &) = delete;

protected:
    noncopyableLog() = default;
    ~noncopyableLog() = default;
};
#endif // ASYNC_LOGGER_noncopyableLog