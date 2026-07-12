#ifndef LOG_H
#define LOG_H

#include <SDL2/SDL.h>

#include <cstdio>

// 初始化日志输出到文件
static inline void InitLogFile(const char* path)
{
    // 追加模式，stdout/stderr 全部重定向到文件
    freopen(path, "a", stdout);
    freopen(path, "a", stderr);
}

// 关闭日志文件
static inline void CloseLogFile()
{
    fclose(stdout);
    fclose(stderr);
}

#endif  // LOG_H