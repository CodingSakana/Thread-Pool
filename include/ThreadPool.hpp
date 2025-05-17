#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

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

    void enqueue(std::function<void()> task);
};