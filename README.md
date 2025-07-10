# 高性能 Linux ThreadPool（C++20 线程池）

<p>
  <a href="#high-performance-linux-threadpool-c20">English version available below – click here to jump</a>
</p>

一款面向 **高频线程创建** 场景的线程池，核心组件包括：

| 组件               | 作用                                              |
| ------------------ | ----------------------------------------------- |
| **ThreadPool**     | 管理工作线程，复用线程执行任务，减少线程创建/销毁开销。 |
| **enqueue**        | 提交任意可调用对象与参数，返回 `std::future` 获取结果。 |
| **workerLoop**     | 工作线程主循环，阻塞等待任务并执行。                    |

---

 **目标**：
 - 减少短任务场景下的线程管理开销；  
 - 提高并发吞吐量与资源利用率；  
 - 提供简洁安全的异步接口。   
 - 短任务场景下相比不使用 **ThreadPool** 获得约 **3x** 的吞吐提升。  

---

## API

```cpp
#include "ThreadPool.hpp"
#include <iostream>

int main() {
    // 创建 4 个工作线程的线程池
    threadpool::ThreadPool pool(4);

    // 提交任务并获取 future
    auto f1 = pool.enqueue([](int a, int b) {
        return a + b;
    }, 2, 3);

    // 异步获取结果
    std::cout << "2 + 3 = " << f1.get() << std::endl;
    return 0;
}

```

---

## 特性

- **C++20 / STL** 实现，依赖极少。
- **完美转发**：支持任意可调用对象与参数，通过 `std::invoke_result_t<>` 推导返回类型。
- **`std::future` 获取结果**：内部基于 `std::packaged_task` 与 `std::future`，异步安全。
- **线程复用**：预先创建固定数量工作线程，重复利用，避免频繁创建/销毁。
- **缓存行对齐**：`stop` 原子标志使用 `alignas(64)`，减少 false sharing。
- **异常传播**：任务抛出异常时，通过 `std::future` 传递至调用者。
- **ASan / TSan** 测试全通过。

---

## 目录结构

```
threadpool/
├─ include/
│   └─ ThreadPool.hpp      // 头文件，声明与模板实现
├─ src/
│   └─ ThreadPool.cpp      // 构造、析构与 workerLoop 实现
├─ tests/
│   └─ performance_test.cpp         // 性能对比测试示例
│   └─ threadpool_full_test.cpp     // 功能验证测试示例
├─ CMakeLists.txt          // CMake 构建脚本
└─ README.md               // 使用说明（本文件）
```

---

## 构建 & 运行

### 1. 生成构建文件

> 项目根目录下运行
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

> 调试模式可改为 `-DCMAKE_BUILD_TYPE=Debug`，会带 `-g` 并关闭优化。

### 2. 编译

```bash
make
```

### 3. 运行单元测试

```bash
make test
```

### 4. 运行性能基准

```bash
make perf
```

---

## 示例性能
> **Intel i7-10875H · 16 线程 ·  WSL2 Ubuntu 24.04.2 LTS · gcc 13.3 -O3**

```
[100%] Built target performance_test
Hardware threads: 16

===== Tasks: 1000 =====
Data size per task: 100000

--- Without thread pool ---
Total time (ms): 47

--- With ThreadPool ---
Total time (ms): 15

[ThreadPool] All worker threads joined. Pool destroyed.
*** Speedup factor: 3.13333x ***

===== Tasks: 10000 =====
Data size per task: 100000

--- Without thread pool ---
Total time (ms): 439

--- With ThreadPool ---
Total time (ms): 146

[ThreadPool] All worker threads joined. Pool destroyed.
*** Speedup factor: 3.00685x ***

===== Tasks: 100000 =====
Data size per task: 100000

--- Without thread pool ---
Total time (ms): 4570

--- With ThreadPool ---
Total time (ms): 1457

[ThreadPool] All worker threads joined. Pool destroyed.
*** Speedup factor: 3.13658x ***

[100%] Built target perf

```

---

## 贡献

欢迎 PR / Issue！

---

**Enjoy efficient concurrency! 🚀**

---
# High-Performance Linux ThreadPool (C++20)

A ThreadPool designed for **high-frequency task** scenarios in C++20, featuring:

| Component    | Purpose                                                      |
| ------------ | ------------------------------------------------------------ |
| **ThreadPool** | Manages worker threads, reuses threads for tasks, minimizing creation/destruction overhead.
| **enqueue**  | Submits any callable with arguments, returns a `std::future` for result retrieval.       |
| **workerLoop** | Worker thread main loop: waits for tasks and executes them.                                  |

--- 

**Goals:**
 - Minimize thread management overhead for short-lived tasks.
 - Maximize concurrency throughput and resource utilization.
 - Provide a simple, safe asynchronous interface.
 - **Benchmarks** show ~**3×** throughput improvement over spawning threads per task in short-task workloads.

---

## API Example

```cpp
#include "ThreadPool.hpp"
#include <iostream>

int main() {
    // Create a ThreadPool with 4 worker threads
    threadpool::ThreadPool pool(4);

    // Submit a task and get a future
    auto f1 = pool.enqueue([](int a, int b) {
        return a + b;
    }, 2, 3);

    // Retrieve the result asynchronously
    std::cout << "2 + 3 = " << f1.get() << std::endl;
    return 0;
}
```

---

## Features

- **C++20 / STL-based:** Minimal external dependencies.
- **Perfect Forwarding:** Supports any callable and arguments with `std::invoke_result_t<>` return-type deduction.
- **`std::future` Results:** Built on `std::packaged_task` + `std::future` for safe asynchronous exception propagation.
- **Thread Reuse:** Fixed pool of worker threads to avoid frequent creation/destruction.
- **Cache-line Alignment:** `stop` flag aligned to 64 bytes to reduce false sharing.
- **Exception Handling:** Exceptions thrown in tasks propagate to caller via `std::future`.
- **Sanitizer Tested:** ASan and TSan clean.

---

## Repository Structure
```
threadpool/
├─ include/
│   └─ ThreadPool.hpp           # Header declarations and inline template
├─ src/
│   └─ ThreadPool.cpp           # Implementation of ThreadPool methods
├─ tests/
│   ├─ performance_test.cpp     # Performance benchmark examples
│   └─ threadpool_full_test.cpp # Functional test suite
├─ CMakeLists.txt               # Build configuration
└─ README.md                    # Usage guide (this file)
```

---

## Build & Run

### 1. Configure
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```
Use `-DCMAKE_BUILD_TYPE=Debug` for debug mode (`-g`, no optimizations).

### 2. Compile
```bash
make
```

### 3. Run Unit Tests
```bash
make test
```

### 4. Run Performance Benchmark
```bash
make perf
```

---

## Sample Performance Results
**Platform:** Intel i7-10875H, 16 threads, WSL2 Ubuntu 24.04 LTS, gcc 13.3 -O3

```
[100%] Built target performance_test
Hardware threads: 16

===== Tasks: 1,000 =====
Data per task: 100,000 loops

--- Without ThreadPool ---
Total time (ms): 47

--- With ThreadPool ---
Total time (ms): 15
[ThreadPool] All worker threads joined. Pool destroyed.
*** Speedup factor: 3.13× ***

===== Tasks: 10,000 =====
Data per task: 100,000 loops

--- Without ThreadPool ---
Total time (ms): 439

--- With ThreadPool ---
Total time (ms): 146
[ThreadPool] All worker threads joined. Pool destroyed.
*** Speedup factor: 3.01× ***

===== Tasks: 100,000 =====
Data per task: 100,000 loops

--- Without ThreadPool ---
Total time (ms): 4570

--- With ThreadPool ---
Total time (ms): 1457
[ThreadPool] All worker threads joined. Pool destroyed.
*** Speedup factor: 3.14× ***
```

---

## Contributions

Contributions, issues, and feature requests are welcome. Feel free to submit a PR or open an issue.

---

**Enjoy efficient concurrency! 🚀**

