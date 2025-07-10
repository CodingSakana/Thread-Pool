#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace threadpool
{

/**
 * 线程池实现，用于管理多个工作线程并调度任务执行
 */
class ThreadPool {
private:
    // 工作线程列表，每个线程会不断从任务队列中获取任务并执行
    std::vector<std::thread> workers;

    // 任务队列，存储所有待执行的任务
    std::queue<std::function<void()>> tasks;

    // 互斥量，用于保护任务队列的并发访问
    std::mutex queueMutex;

    // 条件变量，用于在任务到来时通知工作线程
    std::condition_variable condition;

    // 原子标志，表示线程池是否停止接受新任务并准备关闭
    alignas(64) std::atomic<bool> stop{false}; // cache-line 对齐，避免 false sharing

    // 禁止拷贝、移动
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * 工作线程的执行循环函数
     * 每个工作线程会在此函数中阻塞等待条件变量通知，一旦有任务到来便取出并执行
     */
    void workerLoop();

public:
    /**
     * 构造函数，初始化线程池并启动指定数量的工作线程
     * threadNums 是工作线程的数量
     */
    explicit ThreadPool(size_t threadNums);

    /**
     * 析构函数，通知所有线程停止并回收资源
     */
    ~ThreadPool();

    /**
     * 将可调用对象封装为任务并放入任务队列，返回 future 以获取结果
     */
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;
};

// 模板函数的实现需要放在头文件中，以确保编译时可见
template <typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    // 定义任务返回类型，通过 std::invoke_result_t 推导 F(args...) 的返回值类型
    using return_type = std::invoke_result_t<F, Args...>;

    // 创建一个 shared_ptr，使 lambda 可复制以满足 std::function 约束
    auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
        // 参数包捕获（C++20 特性），将 func 和 params 打包进 lambda，并用 lambda 表达式构造
        // std::packaged_task<> 这样做有以下好处：
        // 1. 保留调用者传入参数的值/引用属性，避免不必要的拷贝或移动。
        // 2. 对于传入的右值，可触发移动语义，提升性能；对于左值，仍按引用使用。
        // 3. 提高模板库的通用性，使其能透明、无损地转发任意可调用对象和参数。
        //
        // mutable 允许 lambda 表达式修改捕获的变量（默认是 const 模式，不可修改）
        [func = std::forward<F>(f), ... params = std::forward<Args>(args)]() mutable {
            return func(params...); // 在工作线程中调用实际的可调用对象并返回结果
        });

    // 获取 future，以便调用者异步获取任务执行结果
    std::future<return_type> future = task_ptr->get_future();

    {
        // 将任务加入队列前获取锁，防止并发冲突
        std::unique_lock<std::mutex> lock(queueMutex);
        // 如果线程池已经停止，则不允许再加入新任务，抛出异常
        if (stop.load(std::memory_order_acquire)) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        // 将封装好的任务放入队列，使用 lambda 调用 (*task_ptr)()
        tasks.emplace([task_ptr]() { (*task_ptr)(); });  // 复制 shared_ptr
    }

    // 通知一个正在等待的工作线程，有新任务到来
    condition.notify_one();
    return future;
}

} // namespace threadpool
