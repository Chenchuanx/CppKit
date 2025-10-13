#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h> // POSIX
#include <memory>

class Client
{
public:
    static Client& getInstance(int port = 8080, const char *serv_ip = "127.0.0.1") {
        // 懒汉模式
        // 线程不安全，不可使用多线程
        // 为了防止创建失败，提供多次创建的机会，使用unique_ptr
        if (!instance_) {
            instance_.reset(new Client(port, serv_ip)); 
        }
        return *instance_;
    }

    void SendMessage(const std::string& message);
    std::string RecvMessage();
    ~Client() { close(fd_); }
private:
    Client(int port, const char *serv_ip);

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    int fd_;
    static std::unique_ptr<Client> instance_;
};

#endif // CLIENT_H