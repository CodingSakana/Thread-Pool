#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threadNums): stop(false){ // 负责创建线程
    for(size_t i = 0; i < threadNums; ++i){
        workers.emplace_back([this](){this->workerLoop();});
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();

    for (std::thread &worker : workers) {
        if (worker.joinable())
            worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task){
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if(stop) return;
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return stop || !tasks.empty(); });  // 后面的表达式为ture时，停止等待.
            /*
            相当于这个：
            while (tasks.empty() && !stop) {
            condition.wait(lock);
            }
            */
            if (stop && tasks.empty()) return;

            task = std::move(tasks.front());
            tasks.pop();
        }

        task();
    }
}

/*
void ThreadPool::workerLoop(){
    std::unique_lock<std::mutex> lock(queueMutex);
    while(true){
        condition.wait(lock, [this](){return !tasks.empty() || stop;}); // 后面的表达式为ture时，停止等待。
        ------
        相当于这个：
        while (tasks.empty() && !stop) {
        condition.wait(lock);
        }
        ------
        if (stop && tasks.empty()) return;
        if(!tasks.empty()){
            auto task = tasks.front();
            tasks.pop();
            lock.unlock();
            task();
        }
        lock.lock();
    }
}
*/