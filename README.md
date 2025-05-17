# ThreadPool

A simple C++20 thread pool implementation from scratch.  
Supports submitting tasks to be executed by a fixed number of worker threads.

---

## ✨ Features

- [x] Fixed-size thread pool
- [x] Asynchronous task submission
- [x] Safe multithreaded task queue
- [x] Graceful shutdown on destruction
- [x] Return value support with `std::future`
- [ ] (Planned) Priority task scheduling

---

## 📁 Project Structure

```
/priority-threadpool
├── include/
│ └── ThreadPool.hpp # Public thread pool API
├── src/
│ └── ThreadPool.cpp # Thread pool implementation
│ └── buggy-v1.5.cpp # buggy code
├── test/
│ └── v1.5-test.cpp # latest test example
│ └── v1.0-test.cpp
│ └── v1.5-test
├── docs/
│ └── 1-threadpool-v1.5-debug-notes.md # 一些bug日志
│ └── 2-forward-capture.md # 一些bug总结
├── CMakeLists.txt # (Coming soon) For future build system
└── README.md
```

---

## 🚀 Build & Run

### Compile manually (Linux / g++):

```bash
g++ -std=c++20 -O3 -Wall -pthread src/ThreadPool.cpp test/v1.5-test.cpp -Iinclude -o test/v1.5-test
./test/v1.5-test
```
Or use VS Code + tasks.json to build.

---

## 🧪 Example Usage

```c++
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
std::cout << "fut1 result: " << fut1.get() << "\n";
std::cout << "fut2 result: " << fut2.get() << "\n";
std::cout << "fut3 result: " << fut3.get() << "\n";

std::cout << "[Main] All done.\n";
return 0;
```
---
## 📌 Future Work

- Support enqueue() with return values using std::future

- Support variable-size dynamic thread pools

- Task prioritization with std::priority_queue

- Benchmark & performance tests

---
## 🧠 Why I Built This

just practicing C++20 skill.

---
## 🧑‍💻 Author
Made by SSSakana.  
Updated on May 17, 2025.
