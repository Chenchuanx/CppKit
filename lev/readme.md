# Levenshtein Distance 莱文斯坦距离
## 参考资料
- 讲解视频：https://www.youtube.com/watch?v=tG4IeY01-xw&list=PLpM-Dvs8t0VbJu4lxZpKaXU6wFL5SApKW
- 开源实现：https://github.com/tsoding/piff

## 简介
莱文斯坦距离用于计算将一个字符串转换为另一个字符串所需的最少编辑操作次数，支持替换、插入、删除三种操作。具体问题可参考LeetCode第72题。
算法时间复杂度：$O(NM)$，$N、M$分别为两个字符串的长度。

## 公式
$$
\operatorname{lev}(a,b) =
\begin{cases}
|a| & \text{if } |b| = 0, \\[4pt]
|b| & \text{if } |a| = 0, \\[4pt]
\operatorname{lev}(\operatorname{tail}(a), \operatorname{tail}(b)) & \text{if } \operatorname{head}(a) = \operatorname{head}(b), \\[8pt]
1 + \min
\begin{cases}
\operatorname{lev}(\operatorname{tail}(a), b) \\
\operatorname{lev}(a, \operatorname{tail}(b)) \\
\operatorname{lev}(\operatorname{tail}(a), \operatorname{tail}(b))
\end{cases}
& \text{otherwise}
\end{cases}
$$
设有两个字符串 $a、b$，长度分别为 $|a|$ 和 $|b|$，二者的莱文斯坦距离记作 $\operatorname{lev}(a,b)$，定义式如上。\
其中，对任意字符串 $x$，$\operatorname{tail}(x)$ 代表去掉首字符后剩余的子串（例：$\operatorname{tail}(x_0x_1\dots x_n) = x_1x_2\dots x_n$）；$\operatorname{head}(x)$ 代表字符串的首字符（例：$\operatorname{head}(x_0x_1\dots x_n) = x_0$）。字符串下标从0开始计数，$x[n]$ 或 $x_n$ 均可表示字符串 $x$ 的第 $n$ 个字符，因此首字符可写作 $x_0$ 或 $x[0]$。\
min函数内第一项对应**删除**操作（将 $a$ 转为 $b$），第二项对应**插入**操作，第三项对应**替换**操作。
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