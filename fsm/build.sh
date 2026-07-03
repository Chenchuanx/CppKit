#!/bin/bash

g++ example.cpp -o example

# 编译成功才运行
if [ $? -eq 0 ]; then
    echo "===== 运行结果 ====="
    ./example
fi