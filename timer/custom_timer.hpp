#include <atomic>
#include <future>
#include <chrono>
#include <thread>
#include <functional>

// 单次自定义任务计时器
class CustomTimer {
public:
    // 构造函数    
    CustomTimer() : is_running(false) {}

    // 析构函数
    ~CustomTimer() {
        stop();
    }

    // 启动定时器
    template<typename F, typename... Arg>
    void start(std::chrono::milliseconds duration, std::chrono::milliseconds interval, 
        F && f, Arg &&... args) {
        stop(); // 确保之前的定时器已停止
        is_running = true;
        // async启动一个线程来处理定时任务
        // 自动管理线程的生命周期，并且不需要显式地创建和管理线程，代码更加简洁
        timer_future = std::async(std::launch::async, [=](){
            try{
                auto start_time = std::chrono::steady_clock::now();
                auto end_time = start_time + duration;  // 结束时间点
                auto next_check = start_time;  // 下次检查时间点

                while (is_running.load() && std::chrono::steady_clock::now() < end_time) {
                    std::this_thread::sleep_until(next_check); // 避免忙等待
                    next_check += interval; // 更新下次检查时间点
                    if (is_running.load()) {
                        std::invoke(f, args...); // 执行回调函数, invoke可运行成员函数
                    }
                }
            } catch (const std::exception& e) {
                is_running = false;
                exception = std::current_exception(); // 捕获异常
                std::cout << "Exception caught in timer: " << e.what() << std::endl;
            }
        });
    }

    // 停止定时器, 手动停止/程序结束
    void stop() {
        is_running = false;
        exception = nullptr; // 清除异常
        if (timer_future.valid()) { // 如果timer_future有效，等待其完成
            timer_future.wait();
        }
    }

    // 检查定时器是否抛出异常
    bool has_exception() const {
        return exception != nullptr;
    }

private:
    std::atomic<bool> is_running;
    std::future<void> timer_future; // 用于管理异步任务(监听定时器是否结束)
    std::exception_ptr exception;   // 用于捕获异常
};
