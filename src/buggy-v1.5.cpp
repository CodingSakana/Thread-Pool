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
    std::packaged_task<return_type()> task([f, args...](){return f(args...);});
    /* 下面这个还没看懂
    auto task = std::packaged_task<return_type()>(
        [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
            return f(std::forward<Args>(args)...);
        });
    */
    auto future = task.get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

        // 将 move-only 的 lambda 显式封装为 std::function<void()>：
        // lambda 捕获了 std::packaged_task，它是不可复制（只能移动）的，
        // 而 std::function 默认要求复制构造。所以必须用 std::move(...) 显式构造 std::function，
        // 否则会编译失败（static_assert: target must be copy-constructible）
        tasks.emplace([t = std::move(task)]() mutable { t(); });
        /*
        mutable:
        因为 t 是通过值捕获的（t = std::move(task)），而 std::packaged_task 的调用是对 *this 的非常量调用（即：operator() 不是 const）。
        所以如果不加 mutable，lambda 会默认是 const 的，调用 t() 会出错（不能对 const 调用 task()）。
        */
    }
    condition.notify_one();
    return future;
}
