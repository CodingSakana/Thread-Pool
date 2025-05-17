# ThreadPool

A simple C++11 thread pool implementation from scratch.  
Supports submitting tasks to be executed by a fixed number of worker threads.

---

## ✨ Features

- [x] Fixed-size thread pool
- [x] Asynchronous task submission
- [x] Safe multithreaded task queue
- [x] Graceful shutdown on destruction
- [ ] (Coming soon) Return value support with `std::future`
- [ ] (Planned) Priority task scheduling

---

## 📁 Project Structure

```
/priority-threadpool
├── include/
│ └── ThreadPool.hpp # Public thread pool API
├── src/
│ └── ThreadPool.cpp # Thread pool implementation
├── test/
│ └── main.cpp # Basic test example
├── CMakeLists.txt # (Coming soon) For future build system
└── README.md
```

---

## 🚀 Build & Run

### Compile manually (Linux / g++):

```bash
g++ src/ThreadPool.cpp test/main.cpp -Iinclude -o test/test-threadpool -Wall -O2
./test/test-threadpool
```
Or use VS Code + tasks.json to build.

---

## 🧪 Example Usage

```c++
ThreadPool pool(4);
for (int i = 0; i < 8; ++i) {
    pool.enqueue([i] {
        std::cout << "Task " << i << " is running.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Task " << i << " is done.\n";
    });
}
```
---
## 📌 Future Work

- Support enqueue() with return values using std::future

- Support variable-size dynamic thread pools

- Task prioritization with std::priority_queue

- Benchmark & performance tests

---
## 🧠 Why I Built This

This project is written to fully understand thread pool internals
and gain confidence in multithreaded system design with C++11+.

---
## 🧑‍💻 Author
Made by SSSakana.  
May 17, 2025.