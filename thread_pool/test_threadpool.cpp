#include <iostream>
#include <vector>
#include "threadpool.h"

std::string task(std::string s)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return s + "\tdone";
}

int main() 
{
    // 创建线程池，指定线程数量
    thread_pool pool(4);

    // 提交任务到线程池
    // lambda 函数
    auto future1 = pool.push_task([](int a, int b) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return a + b;
    }, -5, 6);
    
    // 普通函数
    auto future2 = pool.push_task(task, "2 ");

    // 类成员函数/静态成员函数
    class node {
        int v;
    public:
        node(int v) : v(v) {

        }

        int operator()(int x) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return v + x;
        }

        static int bar(int x) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return x * 2;
        }
    };
    auto future3 = pool.push_task(node{20}, -17);
    auto future4 = pool.push_task(&node::bar, 2);
    // 带有异常的任务
    auto future5 = pool.push_task([]() {
        throw std::runtime_error("error in task");
    });

    // 获取任务结果
    try {
        std::cout << "future1: " << future1.get() << std::endl; // 输出 1
        std::cout << "future2: " << future2.get() << std::endl; // 输出 "2 done"
        std::cout << "future3: " << future3.get() << std::endl; // 输出 -3
        std::cout << "future4: " << future4.get() << std::endl; // 输出 4
    } catch (const std::exception &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    try {
        future5.get(); // 重新抛出任务中的异常
    } catch (const std::exception &e) {
        std::cerr << "future5: 5 " << e.what() << std::endl;
    }

    // 大批量提交任务
    std::vector<std::future<std::string>> results;
    for (int i = 0; i < 50; ++i) {
        results.push_back(pool.push_task(task, "task " + std::to_string(i)));
    }
    for (auto &fut : results) {
        std::cout << fut.get() << std::endl;
    }

    // 性能判断
    {
        const int numTasks = 100;
        std::vector<std::future<std::string>> results;
        auto start = std::chrono::high_resolution_clock::now();
        {
            thread_pool pool(4);
            for (int i = 0; i < numTasks; ++i) {
                results.push_back(pool.push_task(task, "task " + std::to_string(i)));
            }
        }
        auto mid = std::chrono::high_resolution_clock::now();
        for (auto &fut : results) {
            fut.get();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration2 = mid - start;
        std::chrono::duration<double> duration = end - start;
        std::cout << "push_task" << duration2.count() << " seconds, total\n";
        std::cout << "Processed " << numTasks << " tasks in " << duration.count() << " seconds.\n";
    }

    return 0;
}   