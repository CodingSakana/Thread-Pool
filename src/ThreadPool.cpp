#include "ThreadPool.hpp"

namespace threadpool{

ThreadPool::ThreadPool(size_t threadNums) {
    // OPT: 防御式检查
    if (threadNums == 0) {
        throw std::invalid_argument("ThreadPool size cannot be 0");
    }

    for (size_t i = 0; i < threadNums; ++i) {
        workers.emplace_back([this]() { this->workerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop.store(true, std::memory_order_release);
    }
    condition.notify_all();

    for (std::thread& worker : workers) {
        if (worker.joinable()) worker.join();
    }
    std::cout << "[ThreadPool] All worker threads joined. Pool destroyed.\n";
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] {
                return stop.load(std::memory_order_acquire) || !tasks.empty();
            });

            if (stop.load(std::memory_order_acquire) && tasks.empty()) return;

            task = std::move(tasks.front());
            tasks.pop();
        }
        
        task();
    }
}

} // namespace threadpool