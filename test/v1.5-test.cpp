#include "ThreadPool.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

int func(int);

int main() {
    ThreadPool pool(4); // 创建4线程的线程池

    // 简单任务：返回 int
    auto fut1 = pool.enqueue([](int a, int b) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return a + b;
    }, 3, 4);

    // 返回字符串
    auto fut2 = pool.enqueue([](std::string s, int n){
        return s + " " + std::to_string(n);
    }, "task", 2);

    // 调用函数
    auto fut3 = pool.enqueue(func, 10);
    
    // 无返回值任务
    pool.enqueue([]() {
        std::cout << "[Void Task] Hello from thread\n";
    });

    std::cout << "[Main] Waiting for futures...\n";
    std::cout << "fut1 result: " << fut1.get() << "\n"; // 7
    std::cout << "fut2 result: " << fut2.get() << "\n"; // task 2
    std::cout << "fut3 result: " << fut3.get() << "\n"; // I'm func(int a), and I'm printing the a: 10

    std::cout << "[Main] All done.\n";
    return 0;
}

int func(int a){
    std::cout << "[func] I'm func(int a), and I'm printing the a: " << a << std::endl;
    return a;
}
