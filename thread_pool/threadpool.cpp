#include "threadpool.h"

thread_pool::~thread_pool() {
    isStop = true;
    cv.notify_all(); // 通知所有等待的线程
    for (std::thread &thread : threads) {
        if (thread.joinable()) {
            thread.join(); // 等待线程完成
        }
    }
    threads.clear();        // 清空线程数组
    this->clear_queue();    // 清空任务队列
}

void thread_pool::clear_queue() {
    // std::unique_lock<std::mutex> lock(this->mutex);  // 其他线程已经停止，不需要锁
    while (!this->q.empty()) {
        Task task = std::move(this->q.front());
        this->q.pop();
    }
}

void thread_pool::add_thread(int nThreads) {    
    for (int i = 0; i < nThreads; ++i) {
        // vector emplace_back 直接在容器内构造对象，避免不必要的拷贝或移动
        // vector 扩容时，会使用移动构造函数，std::thread 有移动构造函数，安全
        threads.emplace_back([this]() {     // 线程函数
            while (true) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(this->mutex);
                    ++this->nWaiting;
                    this->cv.wait(lock, [this](){ 
                        return this->isStop || !this->q.empty();   // 等待 停止/任务非空
                    });
                    --this->nWaiting;
                    if (this->q.empty()) {
                        return; // 线程池被停止，退出
                    }
                    task = std::move(this->q.front());
                    this->q.pop();
                }
                task(); // 执行任务, function<>自动释放
            }
        });
    }
}