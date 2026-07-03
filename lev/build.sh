#!/bin/bash

g++ lev.cpp -o lev

# 编译成功才运行
if [ $? -eq 0 ]; then
    echo "===== 运行结果 ====="
    ./lev
fi