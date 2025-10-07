#include <iostream>
#include <chrono>
#include "custom_timer.hpp"
#include "event_loop.h"

int main()
{
    // CustomTimer timer;
    // timer.start(std::chrono::milliseconds(5000), std::chrono::milliseconds(1000), [](){
    //     static int count = 0;
    //     std::cout << "Timer tick: " << ++count << std::endl;
    // });
    // std::this_thread::sleep_for(std::chrono::milliseconds(7000)); // 主线程等待7秒
    // timer.stop(); 

    // 多次启动定时器
    // for (int i = 0; i < 3; ++i) {
    //     timer.start(std::chrono::milliseconds(3000), std::chrono::milliseconds(500), [i](){
    //         static int count = 0;
    //         std::cout << "Timer " << i+1 << " tick: " << ++count << std::endl;
    //     });
    //     std::this_thread::sleep_for(std::chrono::milliseconds(4000)); // 主线程等待4秒
    //     timer.stop(); // 停止定时器
    // }

    // 多个定时器实例
    // try {
    //     CustomTimer timer1, timer2;
    //     timer1.start(std::chrono::milliseconds(4000), std::chrono::milliseconds(800), [](){
    //         static int count1 = 0;
    //         std::cout << "Timer 1 tick: " << ++count1 << std::endl;
    //     });
    //     timer2.start(std::chrono::milliseconds(120000), std::chrono::milliseconds(12000), [](){
    //         static int count2 = 0;
    //         std::cout << "Timer 2 tick: " << ++count2 << std::endl;
    //     });
    //     timer1.stop(); // 停止定时器1
    //     timer2.stop(); // 停止定时器2
    //     std::this_thread::sleep_for(std::chrono::milliseconds(7000)); // 主线程等待7秒
    // } catch (const std::exception& e) {
    //     std::cerr << "Exception: " << e.what() << std::endl;
    // }

    // 超长时间定时器
    // CustomTimer long_timer;
    // long_timer.start(std::chrono::hours(1), std::chrono::minutes(10), [](){
    //     static int long_count = 0;
    //     std::cout << "Long Timer tick: " << ++long_count << std::endl;
    // });
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 主线程等待5秒
    // long_timer.stop(); // 停止长时间定时器

    // 定时器执行各种函数
    // CustomTimer timer;
    // timer.start(std::chrono::milliseconds(5000), std::chrono::milliseconds(1000), [](int a){
    //     std::cout << "Timer tick: " << a << std::endl;
    // }, 1);

    // {
    //     class task {
    //     public:
    //         void run(std::string str, int& count) {
    //             count++;
    //             std::cout << "task run: " << str << std::endl;
    //         }
    //     };
    //     int count = 0;
    //     task t;
    //     CustomTimer timerClass;
    //     timerClass.start(std::chrono::milliseconds(5000), std::chrono::milliseconds(1000), 
    //     &task::run, &t, "hello", std::ref(count));
    //     std::this_thread::sleep_for(std::chrono::milliseconds(7000)); // 主线程等待7秒
    //     std::cout << "count = " << count << std::endl;
    // }
    // timer.start(std::chrono::milliseconds(5000), std::chrono::milliseconds(1000), [](){
    //     throw std::runtime_error("Something error");
    // });
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 主线程等待7秒

    EventLoop event;
    event.add_task_after(std::chrono::milliseconds(5000), []() {
        std::cout << "Event loop task 3" << std::endl;
    });
    event.start();
    event.add_task_after(std::chrono::milliseconds(3000), []() {
        std::cout << "Event loop task 2" << std::endl;
    });
    event.add_task(std::chrono::steady_clock::now() + std::chrono::milliseconds(6000), []() {
        std::cout << "Event loop task 4" << std::endl;
    });
    event.add_task_after(std::chrono::milliseconds(2000), []() {
        std::cout << "Event loop task 1.1" << std::endl;
    });
        event.add_task_after(std::chrono::milliseconds(2000), []() {
        std::cout << "Event loop task 1.2" << std::endl;
    });


    std::this_thread::sleep_for(std::chrono::milliseconds(7000)); // 主线程等待7秒

    return 0;   
}