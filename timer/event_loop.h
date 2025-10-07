#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <queue>

// 事件循环类
// 没有跟踪事件的能力 1. 异常处理； 2. 返回值； 3. 事件返回值； 4. 取消事件
class EventLoop {
public:
    EventLoop() : is_running(false) {}
    ~EventLoop() {
        this->stop();
    }

    // 启动事件循环
    void start() {
        is_running = true;
        event_loop_thread = std::thread([this]() {
            std::unique_lock<std::mutex> lock(mutex);
            while (is_running) {
                cv.wait(lock, [this]() {
                    return !is_running || !tasks.empty(); // 如果任务队列不为空，则返回true，继续执行任务
                });
                auto now = std::chrono::steady_clock::now();
                while (!tasks.empty() && tasks.top().time <= now) {
                    auto task = tasks.top().func;
                    tasks.pop();
                    lock.unlock();
                    task(); // 执行任务
                    lock.lock();
                }
                if (!tasks.empty()) {
                    cv.wait_until(lock, tasks.top().time); // 等待直到下一个任务的时间点，避免忙等
                }
            }
        });
    }

    // 停止事件循环
    void stop() {
        is_running = false;
        cv.notify_all();
        if (event_loop_thread.joinable()) {
            event_loop_thread.join();
        }
    }

    // 添加任务到任务队列
    void add_task(const std::chrono::steady_clock::time_point& time, std::function<void()> func) {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.push({ time, func });
        cv.notify_all();
    }

    // 在指定延迟后添加任务
    void add_task_after(const std::chrono::milliseconds& delay, std::function<void()> func) {
        add_task(std::chrono::steady_clock::now() + delay, func);
    }

private:
    struct Task {
        std::chrono::steady_clock::time_point time; // 任务执行时间
        std::function<void()> func; // 任务函数
        bool operator< (const Task& other) const {
            return time > other.time; // 优先队列按时间从小到大排序
        }
    };
    std::atomic<bool> is_running;
    std::thread event_loop_thread;

    std::priority_queue<Task> tasks;    // 任务队列(优先队列)

    
    std::mutex mutex;   // 保护任务队列
    std::condition_variable cv;     
};