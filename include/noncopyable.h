#ifndef KIQSONT_MUDUO_COPY_NONCOPYABLE
#define KIQSONT_MUDUO_COPY_NONCOPYABLE
class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
    noncopyable(noncopyable &&) = default;
    noncopyable &operator=(noncopyable &&) = default;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
#endif // KIQSONT_MUDUO_COPY_NONCOPYABLE
