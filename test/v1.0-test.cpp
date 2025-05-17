#include "ThreadPool.hpp"
#include <chrono>

int main() {
    {
        ThreadPool pool(4);
        for (int i = 0; i < 10; ++i) {
            pool.enqueue([i] () {
                std::cout << "Task " << i << " running.\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "Task " << i << " ended.\n";
                return i + 1;
            });
        }
    } // 离开作用域自动析构，观察是否线程安全退出
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Thread Pool exited.\n";
    return 0;
}