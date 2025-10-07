#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

// 线程池 不可调整大小
// 未使用智能指针，当出现异常时，可能会内存泄漏
class thread_pool final {
public:
    using Task = std::function<void()>; // 任务类型

    // 构造函数
    thread_pool(int nThreads) : isStop(false),nWaiting(0) { 
        this->add_thread(nThreads); 
    }

    // 析构函数
    ~thread_pool();

    // 添加任务到线程池
    // @param f 任务函数 
    // @param args 任务函数参数
    // @return std::future 任务结果的 future 对象
    template<typename F, typename... Args>
    auto push_task(F && f, Args&&... args) ->std::future<decltype(f(args...))> {
        if (isStop) {
            throw std::runtime_error("push on stopped thread pool");
        }
        // 使用 std::bind 绑定参数，创建一个无参的包装函数；
        // shared_ptr 用于在任务执行自动释放 packaged_task
        // 若不使用packaged_task是栈上对象，lambda捕获栈上对象会出问题
        auto packgedTask = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        // 包装为统一的 Task(function<>) 类型，不使用new 避免手动释放
        Task task = [packgedTask]() { 
            (*packgedTask)();   
        }; 
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            q.push(std::move(task));
            cv.notify_one(); // 通知一个等待的线程
        }
        return packgedTask->get_future(); // 返回 future
    }

    // 获取线程池中线程数量
    int size() { 
        return static_cast<int>(threads.size());  // size_t -> int
    }

    // 获取空闲线程数量
    int n_idle() { 
        return nWaiting; 
    }

private:
    // 清空任务队列
    void clear_queue();

    // 添加线程
    void add_thread(int nThreads);

    // 禁止拷贝和移动
    thread_pool(const thread_pool &) = delete;
    thread_pool(thread_pool &&) = delete;
    thread_pool & operator=(const thread_pool &) = delete;
    thread_pool & operator=(thread_pool &&) = delete;

    std::vector<std::thread> threads;   // 线程数组
    std::queue<Task> q; // 任务队列

    std::mutex mutex;  // 互斥锁
    std::condition_variable cv; // 条件变量(保护任务队列)

    std::atomic<bool> isStop; // 是否停止线程池
    std::atomic<int> nWaiting; // 等待线程数
};
