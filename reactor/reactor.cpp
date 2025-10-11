#include "event.h"
#include "reactor.h"

// 主循环：等待事件并处理
void Reactor::Run() {
    epoll_event ep_events[1024];
    while (true) {
        int n = epoll_wait(epoll_fd_, ep_events, 1024, -1);
        if (n == -1) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            throw std::runtime_error("epoll_wait failed");
        }
        for (int i = 0; i < n; ++i) {
            auto event = static_cast<Event*>(ep_events[i].data.ptr);
            event->CallBack();
        }
    }
}

// 添加事件到epoll实例
void Reactor::AddEvent(int fd, int events, int status) {
    auto event = std::make_shared<Event>(fd, events, this, status);

    epoll_event ev{0, {0}};
    ev.events = events;
    ev.data.ptr = event.get();

    int ret = epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1) {
        throw std::runtime_error("epoll_ctl add failed: " + std::string(strerror(errno)));
    }

    events_map_.emplace(fd, event);
}

// 删除事件
void Reactor::DelEvent(int fd) {
    // 检查fd是否存在于映射表中
    auto it = events_map_.find(fd);
    if (it == events_map_.end()) {
        return;
    }

    // 从active_fds_中移除fd
    active_fds_.erase(fd);

    // 从epoll中移除事件
    epoll_event ev{};
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
        if (errno != ENOENT) {
            throw std::runtime_error("epoll_ctl del failed: " + std::string(strerror(errno)));
        }
    }

    // 即使关闭失败，也继续清理映射表
    if (close(fd) == -1) {
        std::cerr << "warning: close fd " << fd << " failed: " << strerror(errno) << std::endl;
    }
    events_map_.erase(it);

    std::cout << "event deleted: fd=" << fd << std::endl;
}

// 广播
void Reactor::Broadcast(int fd, std::string recv_data) {
    for (auto& active_fd  : active_fds_) {
        if (active_fd != fd) {
            write(active_fd, recv_data.c_str(), recv_data.size());
        }
    }
}

// 设置活跃的客户端连接
void Reactor::AddActiveFd(int fd) {
    active_fds_.insert(fd);
}

// 创建epoll实例
void Reactor::epoll_create() {
    this->epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);     // 防止子进程持有epoll_fd
    if (this->epoll_fd_ == -1) {
        throw std::runtime_error("epoll_create1 error");
    }
}

// 创建监听器
void Reactor::acceptor_create(int port) {
    // 绑定端口
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1) {
        throw std::runtime_error("socket error");
    }

    // 设置非阻塞
    int flag = fcntl(listen_fd_, F_SETFL, O_NONBLOCK);
    if (flag == -1) {
        close(listen_fd_);
        throw std::runtime_error("fcntl nonblock failed: " + std::string(strerror(errno)));
    }

    // 允许端口复用，避免重启服务时端口占用
    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        close(listen_fd_);
        throw std::runtime_error("bind failied: " + std::string(strerror(errno)));
    }

    // 设置监听数量
    ret = listen(listen_fd_, 1024);
    if (ret == -1) {
        close(listen_fd_);
        throw std::runtime_error("listen failed: " + std::string(strerror(errno)));
    }

    // 将监听器加入epoll实例
    this->AddEvent(listen_fd_, EPOLLIN | EPOLLET, ACCEPTOT);
    active_fds_.erase(listen_fd_);

    std::cout << "Acceptor init success: listen on port " << port << std::endl;
}

Reactor::~Reactor() {
    active_fds_.clear();
    for (auto& event : events_map_) {
        if (close(event.first) == -1) {
            std::cerr << "Warning: Failed to close epoll_fd: " << strerror(errno) << std::endl;
        }
    }
    events_map_.clear();    // sherd_ptr自动释放
    if (close(epoll_fd_) == -1) {
        std::cerr << "Warning: Failed to close epoll_fd: " << strerror(errno) << std::endl;
    }
    if (listen_fd_ != -1 && close(listen_fd_) == -1) { // 检查 listen_fd_ 是否有效
        std::cerr << "Warning: Failed to close listen_fd: " << strerror(errno) << std::endl;
    }
}
