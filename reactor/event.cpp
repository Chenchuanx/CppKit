#include "event.h"
#include "reactor.h"

void Event::CallBack() {
    if (events_ & EPOLLIN) {
        switch(user_.status) {
            case ACCEPTOT:
                Accept();
                break;
            case UNLOGIN:
                Login();
                break;
            case LOGINED:
                Chat();
                break;
        }
    }
}

// 处理新连接
void Event::Accept() {
    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    // ET触发，需要循环accept
    while (true) {
        // accept4 直接将客户端 socket 设为非阻塞
        int cfd = accept4(
            fd_, 
            (struct sockaddr*)&client_addr, 
            &client_addr_len, 
            SOCK_NONBLOCK  // 客户端 socket 设为非阻塞
        );
        if (cfd == -1) {
            // EAGAIN 和 EINTR 阻塞操作遇到了非阻塞fd，表示没有更多连接。
            if (errno != EAGAIN && errno != EINTR) {
                throw std::runtime_error("accept4 error");
            }
            break;
        }

        // 打印新连接信息（可选）
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        int client_port = ntohs(client_addr.sin_port);
        std::cout << "New connection: ip=" << client_ip << ", port=" << client_port << ", fd=" << cfd << std::endl;

        // 将新连接加入epoll实例
        reactor_->AddEvent(cfd, EPOLLIN | EPOLLET, UNLOGIN);
        write(cfd, "Please enter your name: ", 25);
    }
}

// 登录，即输入简单信息
void Event::Login() {
    char buf[1024] = {0};
    ssize_t len = read(fd_, buf, sizeof(buf) - 1);
    if (len == 0) {  // 连接关闭
        reactor_->DelEvent(fd_);
        return ;
    } else if (len < 0) {
        throw std::runtime_error("read error" + std::string(strerror(errno)));
    }
    buf[len - 1] = '\0';    // 补丁: 去掉回车
    user_.name = buf;
    user_.status = LOGINED;
    reactor_->AddActiveFd(fd_);
    reactor_->Broadcast(fd_, "[System]: " + user_.name + " join the chat room\n");
}   

// 聊天
void Event::Chat() {
    std::string recv_data = "[" + user_.name + "]: "; 
    while (true) {  // ET 循环读
        char buf[1024] = {0};
        ssize_t len = read(fd_, buf, sizeof(buf));
        if (len == 0) {  // 连接关闭
            reactor_->DelEvent(fd_);
            return ;
        } else if (len < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                break;
            }
            throw std::runtime_error("read error" + std::string(strerror(errno)));
        }
        recv_data += std::string(buf, len);
    }
    reactor_->Broadcast(fd_, recv_data);
}
