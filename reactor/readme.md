参考：
```
https://github.com/senyucci/TcpChatRoom/tree/main
https://github.com/Aetherance/Chatroom
https://github.com/jwzh222/epoll/
```
完成reactor 极简聊天室。
#### 项目描述
1. 服务端部分
- Reactor类，管理epoll相关内容，包括epoll_create, epoll_ctl, epoll_wait等操作，保留Envent*到epoll_event的ptr中, 监听到通知直接执行Envent的成员函数。
- Event类，保存事件信息，包括fd，events，status与回调函数，通过status的不同执行不同函数。
2. 客户端部分：
- Client类，管理socket连接，包括connect, send, recv等操作。
- 其他：Qt实现的客户端部分， 用于显示消息。

#### 流程描述
1. 编译运行程序后，自动完成epoll和监听fd的初始化, 监听fd对应的Evnent对象status为ACCEPTOR，回调函数执行Accept()。
2. 启动客户端需要，进入 client 文件夹，依次执行以下命令：
```bash
qmake client.pro      # 生成 Makefile
make                  # 编译生成可执行文件
./client              # 运行客户端程序
```
客户端包含两个页面：
- 登录页面：输入用户名并连接服务器
- 聊天页面：收发消息
