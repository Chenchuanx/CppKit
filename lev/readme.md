## 参考资料
- 讲解视频：https://www.youtube.com/watch?v=tG4IeY01-xw&list=PLpM-Dvs8t0VbJu4lxZpKaXU6wFL5SApKW
- 开源实现：https://github.com/tsoding/piff

# Levenshtein Distance 莱文斯坦距离
莱文斯坦距离用于计算将一个字符串转换为另一个字符串所需的最少编辑操作次数，支持替换、插入、删除三种操作。具体问题可参考LeetCode第72题。
算法时间复杂度：$O(NM)$，$N、M$分别为两个字符串的长度。

## 公式
$$
lev(a,b) =
\begin{cases}
|a| & \text{if } |b| = 0, \\
|b| & \text{if } |a| = 0, \\
lev\big(tail(a), tail(b)\big) & \text{if } head(a) = head(b), \\
1 + \min
\begin{cases}
lev\big(tail(a), b\big) \\
lev\big(a, tail(b)\big) \\
lev\big(tail(a), tail(b)\big)
\end{cases}
& \text{otherwise}
\end{cases}
$$

设有两个字符串 $a、b$，长度分别为 $|a|$ 和 $|b|$，二者的莱文斯坦距离记作 $\mathrm{lev}(a,b)$，定义式如上。\
其中，对任意字符串 $x$， $\mathrm{tail}(x)$ 代表去掉首字符后剩余的子串（例： $\mathrm{tail}(x_0x_1\dots x_n) = x_1x_2\dots x_n$ ）； $\mathrm{head}(x)$ 代表字符串的首字符（例： $\mathrm{head}(x_0x_1\dots x_n) = x_0$ ）。字符串下标从0开始计数， $x[n]$ 或 $x_n$ 均可表示字符串 $x$ 的第 $n$ 个字符，因此首字符可写作 $x_0$ 或 $x[0]$。\
 $\mathrm{min}$ 函数内第一项对应**删除**操作（将 $a$ 转为 $b$），第二项对应**插入**操作，第三项对应**替换**操作。\
该数学定义可以直接转化为朴素递归实现代码。

## 三种实现思路演进
### 1. 暴力递归 lev_dfs
按照莱文斯坦公式并使用递归实现，相同`(n1, n2)`组合会重复计算，字符串较长时效率差，易出现递归深度溢出。

### 2. 记忆化递归 lev_cache
增加二维缓存`cache`，`cache[n1][n2]`保存`(n1, n2)`组合的函数返回值。每次递归先查询缓存，存在结果直接返回，消除重复计算

### 3. 迭代动态规划lev_dp
`cache[i][j]`也可被看作`s1前你个字符转为s2前j个字符`的最小操作步数。缓存数组本质就是动态规划状态表，可以舍弃递归，使用双层循环直接填充状态表。

### 总结
综上，我们从基础递归公式一步步改写出动态规划，这是一个很巧妙的思路。

# 命令行Diff
在完成莱文斯坦距离后，使用其来完成：
1. diff比较两个文件差异(Linux的diff工具实际使用Myers算法实现)
2. 根据错误命令推测实际命令\

使用`g++ -std=c++11 -o main main.cpp`即可编译，运行结果如下:
```
chenchuan@U22:~/CppKit/lev/diff$ ./main difff
找不到命令 "difff" 您的意思可能是: "diff"
```
```
chenchuan@U22:~/CppKit/lev/diff$ ./main diff file1.txt file2.txt
--- file1.txt
+++ file2.txt
+[新行1]
-在本次 C++ 代码工程中，我完整实现了三套递进式莱文斯坦距离代码：无缓存暴力递归、带二维缓存记忆递归、无递归迭代 DP，额外拓展了带操作轨迹回溯的 DP版本，可以完整打印字符串每一步转换过程，直观展示替换、删除、插入的执行流程。
+在本次 C++ 代码工程中，[修改]我完整实现了三套递进式莱文斯坦距离代码：无缓存暴力递归、带二维缓存记忆递归、无递归迭代 DP，额外拓展了带操作轨迹回溯的 DP 版本，可以完整打印字符串每一步转换过程，直观展示替换、删除、插入的执行流程。
-[删除]
```