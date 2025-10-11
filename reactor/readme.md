参考：
```
https://github.com/senyucci/TcpChatRoom/tree/main
https://github.com/Aetherance/Chatroom
https://github.com/jwzh222/epoll/
```
完成reactor 极简聊天室。
#### 项目描述
1. Reactor类，管理epoll相关内容，包括epoll_create, epoll_ctl, epoll_wait等操作，保留Envent*到epoll_event的ptr中, 监听到通知直接执行Envent的成员函数。
2. Event类，保存事件信息，包括fd，events，status与回调函数，通过status的不同执行不同函数。
#### 流程描述
1. 运行程序后，自动完成epoll和监听fd的初始化, 监听fd对应的Evnent对象status为ACCEPTOR，回调函数执行Accept()。
2. 使用linux 命令：`nc 127.0.0.1 8080`作为客户端连接，会创建一个Event对象，其status为UNLOGIN，回调函数执行Login(), 执行回调函数后，status变为LOGINED，回调函数会执行Chat()