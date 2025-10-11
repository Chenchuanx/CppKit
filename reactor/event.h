#pragma

#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Reactor;

enum {
    ACCEPTOT= 0,
    UNLOGIN,
    LOGINED
};

class Event {
public:
    Event(int fd, int events, Reactor* reactor ,int status = UNLOGIN) 
        : fd_(fd)
        , events_(events)
        , reactor_(reactor)
        , user_({status, ""})
        {}
    
    void CallBack(); // 回调函数
private:
    // 处理新连接
    void Accept();
    
    // 登录，即输入简单信息
    void Login();

    // 聊天
    void Chat();

    int fd_;
    int events_;
    struct user_info {
        int status;
        std::string name;
    } user_; // 用户数据
    Reactor* reactor_;
};