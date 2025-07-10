// ThreadPool_functional_test.cpp

#include <cassert>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
#include <atomic>
#include "ThreadPool.hpp"

using namespace threadpool;

// ------------------------------------------------------------
// 1. 基础返回值测试
void test_basic() {
    ThreadPool pool(4);
    auto f1 = pool.enqueue([] { return 21 + 21; });
    auto f2 = pool.enqueue([](std::string s) { return s + " world"; }, std::string("hello"));
    assert(f1.get() == 42);
    assert(f2.get() == "hello world");
    std::cout << "[PASS] basic return value\n";
}

// ------------------------------------------------------------
// 2. 并行求和测试
void test_parallel_sum() {
    const size_t N = 10'000;
    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 1);

    ThreadPool pool(3);
    const size_t chunk = 1'000;
    std::vector<std::future<long long>> futures;

    for (size_t i = 0; i < N; i += chunk) {
        futures.emplace_back(pool.enqueue([&, i] {
            long long sum = 0;
            for (size_t j = i; j < std::min(i + chunk, N); ++j)
                sum += data[j];
            return sum;
        }));
    }

    long long total = 0;
    for (auto& f : futures) total += f.get();
    assert(total == static_cast<long long>(N) * (N + 1) / 2);
    std::cout << "[PASS] parallel sum\n";
}

// ------------------------------------------------------------
// 3. 异常传播测试
void test_exception() {
    ThreadPool pool(2);
    auto fut = pool.enqueue([] { throw std::runtime_error("boom"); return 0; });
    bool threw = false;
    try {
        fut.get();
    } catch (const std::runtime_error& e) {
        threw = (std::string(e.what()) == "boom");
    }
    assert(threw);
    std::cout << "[PASS] exception propagation\n";
}

// ------------------------------------------------------------
// 4. 捕获语义测试（值捕获，消除竞态） 
void test_capture() {
    ThreadPool pool(2);

    int x = 5;
    // 值捕获：lambda 内部保存 x 的副本，不受外部修改影响
    auto f1 = pool.enqueue([x] { return x * 2; });
    // 即使在 enqueue 之后修改 x，也不会影响 lambda 内部的值
    x = 7;
    assert(f1.get() == 10);  // 原值 5×2 == 10

    std::string blob(500, 'a');
    auto f2 = pool.enqueue([s = std::move(blob)] { return s.size(); });
    assert(f2.get() == 500);
    assert(blob.empty());
    std::cout << "[PASS] capture semantics\n";
}

// ------------------------------------------------------------
// 5. 析构等待全部任务完成
void test_destructor_completion() {
    std::atomic<int> counter{0};
    {
        ThreadPool pool(4);
        for (int i = 0; i < 100; ++i) {
            pool.enqueue([&counter] { counter.fetch_add(1, std::memory_order_relaxed); });
        }
    }  // ThreadPool 析构时应等待所有任务完成
    assert(counter.load() == 100);
    std::cout << "[PASS] destructor waits\n";
}

// ------------------------------------------------------------
// 6. 0 线程构造必须抛异常
void test_zero_thread() {
    bool threw = false;
    try {
        ThreadPool pool(0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
    std::cout << "[PASS] zero-thread construction\n";
}

// ------------------------------------------------------------
int main() {
    test_basic();
    test_parallel_sum();
    test_exception();
    test_capture();
    test_destructor_completion();
    test_zero_thread();

    std::cout << "All functional ThreadPool tests passed!\n";
    return 0;
}
