#!/bin/bash

g++ main.cpp event.cpp reactor.cpp -o server

# 编译成功才运行
if [ $? -eq 0 ]; then
    echo "===== 运行结果 ====="
    ./server
fi