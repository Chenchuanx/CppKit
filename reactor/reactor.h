#pragma

#include <set>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

class Event;

class Reactor {
public:
    // 构造函数
    Reactor(int port) {
        this->epoll_create();    // 创建epoll实例
        this->acceptor_create(port);  // 创建监听器
    }

    ~Reactor();

    // 主循环：等待事件并处理
    void Run();

    // 添加事件到epoll实例
    void AddEvent(int fd, int events, int status);

    // 删除事件
    void DelEvent(int fd);

    // 广播
    void Broadcast(int fd, std::string recv_data);

    // 设置活跃的客户端连接
    void AddActiveFd(int fd);

private:
    // 创建epoll实例
    void epoll_create();

    // 创建监听器
    void acceptor_create(int port);

    int epoll_fd_;
    int listen_fd_;
    std::set<int> active_fds_;  // 活跃的客户端连接
    std::unordered_map<int, std::shared_ptr<Event>> events_map_;
};
