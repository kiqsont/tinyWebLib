#ifndef KIQSONT_MUDUO_COPY_CALLBACKS
#define KIQSONT_MUDUO_COPY_CALLBACKS

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
using TimerCallback = std::function<void()>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

#endif // KIQSONT_MUDUO_COPY_CALLBACKS