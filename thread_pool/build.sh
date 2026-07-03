#!/bin/bash

g++ threadpool.cpp test_threadpool.cpp -o test

# 编译成功才运行
if [ $? -eq 0 ]; then
    echo "===== 运行结果 ====="
    ./test
fi