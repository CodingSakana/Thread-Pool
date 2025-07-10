// test_threadpool.cpp
#include "ThreadPool.hpp" // 你的线程池头文件
#include <chrono>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

// 模拟计算密集型任务：对长度为 N 的 vector 求和
long compute_sum(std::size_t N) {
    std::vector<int> data(N, 1);
    // 简单累加
    return std::accumulate(data.begin(), data.end(), 0L);
}

int main() {
    // 不同的任务总数，用于对比
    const std::vector<std::size_t> taskCounts = {1'000, 10'000, 100'000};
    const std::size_t dataSize = 100'000;                                // 每个任务的数据量
    const std::size_t threadCount = std::thread::hardware_concurrency(); // 可用硬件线程数

    // 英文输出硬件线程数
    std::cout << "Hardware threads: " << threadCount << "\n\n";

    // 对每个 taskCount 执行对比测试
    for (auto taskCount : taskCounts) {
        std::cout << "===== Tasks: " << taskCount << " =====\n";
        std::cout << "Data size per task: " << dataSize << "\n\n";

        // 1. Without thread pool
        long long duration_no_pool_ms = 0;
        {
            std::cout << "--- Without thread pool ---\n";
            auto t0 = std::chrono::high_resolution_clock::now();

            // 每个任务都创建一个线程
            std::vector<std::thread> threads;
            threads.reserve(taskCount);
            for (std::size_t i = 0; i < taskCount; ++i) {
                threads.emplace_back([dataSize]() {
                    volatile long result = compute_sum(dataSize);
                    (void)result; // 防止优化掉
                });
            }
            // 等待所有线程完成
            for (auto& t : threads) {
                t.join();
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            duration_no_pool_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            std::cout << "Total time (ms): " << duration_no_pool_ms << "\n\n";
        }

        // 2. With ThreadPool
        long long duration_with_pool_ms = 0;
        {
            std::cout << "--- With ThreadPool ---\n";
            // 创建指定大小的线程池
            threadpool::ThreadPool pool(threadCount);

            auto t0 = std::chrono::high_resolution_clock::now();

            // 提交任务并保存 future
            std::vector<std::future<long>> futures;
            futures.reserve(taskCount);
            for (std::size_t i = 0; i < taskCount; ++i) {
                futures.emplace_back(pool.enqueue(compute_sum, dataSize));
            }
            // 等待所有任务完成
            for (auto& f : futures) {
                f.get();
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            duration_with_pool_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            std::cout << "Total time (ms): " << duration_with_pool_ms << "\n\n";
        }

        // 3. Calculate speedup
        double speedup = static_cast<double>(duration_no_pool_ms) / duration_with_pool_ms;
        std::cout << "*** Speedup factor: " << speedup << "x ***\n\n";
    }

    return 0;
}
