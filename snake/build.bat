@echo off
:: 自动创建输出文件夹
if not exist bin\release mkdir bin\release
:: 复制全部SDL dll到exe目录
:: copy "D:\project\game\SDL2_w64\bin\*.dll" bin\release\

:: 编译源码
g++ -c *.cpp -std=c++11 -O3 -Wall -m64 -I include -I D:\project\game\SDL2_w64\include
:: 链接生成exe
g++ *.o -o bin/release/snake.exe -s -L D:\project\game\SDL2_w64\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
:: 删除临时.o文件
del *.o
:: 启动游戏
start bin/release/snake.exe