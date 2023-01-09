#ifndef KIQSONT_MUDUO_OVERWRITE_NONOVERWRITEABLE
#define KIQSONT_MUDUO_OVERWRITE_NONOVERWRITEABLE
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
#endif // KIQSONT_MUDUO_OVERWRITE_NONOVERWRITEABLE
