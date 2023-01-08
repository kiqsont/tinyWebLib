#ifndef ASYNC_LOGGER_NONCOPYABLE
#define ASYNC_LOGGER_NONCOPYABLE
class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
#endif // ASYNC_LOGGER_NONCOPYABLE