#include "ThreadPool.hpp"
#include <iostream>
#include <chrono>

int main() {
    ThreadPool pool(4); // 创建4个线程

    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " is running in thread "
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));   // 等一会儿任务做完
    return 0;
}