# C++ Tiny Web Library


| Part Ⅰ | Part Ⅱ| Part Ⅲ| Part Ⅳ |
| :----: | :----:| :----:| :----:   |
| [项目背景](#项目背景) | [项目特点](#项目特点) | [压力测试](#压力测试) | [运行案例](#运行案例) |



项目背景
----------

本项目是参考muduo的项目结构，使用C++17进行改写，目的是去除boost库的依赖，同时使用部分modern C++的特性，让该网络库使用起来更加方便，而源码的阅读也更加容易。

本项目实现了 Channel模块，EventLoop模块，Poller模块，定时器模块，HTTP服务模块，同时独立出了异步日志库，还增加了openssl的支持以实现HTTPS等安全连接。



项目特点
----

- 使用 Epoll 的 LT模式，配合 IO多路复用，实现非阻塞 IO的主从Reactor模型
- 使用 one loop per thread 的线程模型理念，支持线程池管理的 EventLoop
- 独立出了异步日志库，使用fmt库的格式，日志数据能通过后台线程刷入到本地文件,也可以输出到控制台
- 使用基于红黑树的std::set去管理定时器队列，结合Linux的timerfd实现定时任务功能
- 尽可能使用智能指针管理内存资源，符合 RAII 理念，减小内存泄漏风险
- 使用状态模式，以有限状态机形式解析 HTTP 请求报文
- 结合openssl库，实现TLS安全通信功能



压力测试
----

在 i5-10200H 双核 4G内存的VMware下

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/webbench.png)





运行案例
----
项目的构建
```bash
# 在tinyWebLib目录下
./autobuild.sh
```
所有的测试程序都在 example/bin 目录下，建议在bin目录下运行

- Echo 服务器

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/echo_test.png)

- time 服务器

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/time_test.png)

- 定时器使用案例

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/timer_test.png)

- 时间轮，定时踢掉空闲连接(3s)

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/timingWheel.png)

- HTTP服务器

支持静态文件发送，GET/POST请求接口

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/http_server.png)
接口编写便捷
![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/http_interface.png)

- HTTPS服务器

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/https_server.png)

- http程序性能的火焰图(svg文件在项目介绍/)

![](./%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D/http_flamegraph.png)
