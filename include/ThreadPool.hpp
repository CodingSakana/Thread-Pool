#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <iostream>

class ThreadPool{
private:
    std::vector<std::thread> workers;            // 工作线程列表
    std::queue<std::function<void()>> tasks;    // 任务队列
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    void workerLoop();
    
public:
    ThreadPool(size_t threadNums);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
         -> std::future<std::invoke_result_t<F, Args...>>;
};

template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>
{
    using return_type = std::invoke_result_t<F, Args...>;

    auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
        std::packaged_task<return_type()>(
            [func = std::forward<F>(f),
             ... params = std::forward<Args>(args)] () mutable
            {
                return func(params...);
            }));    // C++20 参数包捕获

    std::future<return_type> future = task_ptr->get_future();

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

        // shared_ptr 是可拷贝的 → std::function OK
        tasks.emplace([task_ptr]() { (*task_ptr)(); });
    }
    condition.notify_one();
    return future;
}
