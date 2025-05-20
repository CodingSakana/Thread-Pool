# ThreadPool v1.5 调试与架构记录

日期：2025-05-17  
作者：SSSakana

---

## 问题背景

实现一个支持返回值任务（std::future）的线程池，即 `enqueue()` 支持传入任意可调用对象 `F` 及其参数 `Args...`，并返回 `std::future<ReturnType>`。

---

### 问题 1：lambda 捕获 packaged_task 无法被 std::function 存储

```text
error: static assertion failed: std::function target must be copy-constructible
```

**原因**：lambda 捕获了 `std::packaged_task`（move-only），而 `std::function` 默认要求 **可拷贝对象**。

**尝试失败的修复**：
- 尝试用 `std::function<void()>([t=std::move(task)](){ t(); })` → 仍然报错
- 尝试 `emplace(...)` → 仍然失败

**最终解决**：
- 将 `task` 包装成 `std::shared_ptr<std::packaged_task<...>>`
- lambda 捕获 `shared_ptr`，从而变成 **可拷贝**

```cpp
auto task_ptr = std::make_shared<std::packaged_task<...>>( ... );
tasks.emplace([task_ptr]{ (*task_ptr)(); });
```

---

### 问题 2：参数包捕获（`... params = std::forward<Args>(args)`）报错

```text
warning: pack init-capture only available with ‘-std=c++20’
error: no matching function for call to ‘forward<const char(&)[5]>(const char*&)’
```

**原因**：
- 参数包初始化捕获语法是 **C++20** 才支持的；
- 字符串字面量 `"task"` 退化成 `const char*`，不能再用 `std::forward<Args>` 推导为数组引用类型。

**解决**：
- 将编译器切换为 `-std=c++20`
- lambda 中调用时 **去掉 forward**：`return func(params...)`

---
## 关于参数包捕获的详细问题见 2-forward-capture
