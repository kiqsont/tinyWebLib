
#include "EventLoop.h"
#include <functional>
#include <iostream>

using std::cout;

int main()
{
    EventLoop loop;
    int num = 0;
    loop.runEvery(1, [&num]()
                  { cout << ++num << "s\n"; });
    loop.runAfter(5, [&loop]()
                  { cout << "timer finish\n"; loop.quit(); });

    loop.loop();
}