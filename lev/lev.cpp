#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// 直接递归
int lev_dfs(const string &s1, const string &s2, int n1, int n2)
{
    if (n1 == 0) return n2;
    if (n2 == 0) return n1;
    if (s1[n1 - 1] == s2[n2 - 1])
        return lev_dfs(s1, s2, n1-1, n2-1);         // 忽略
    return 1 + min({lev_dfs(s1, s2, n1-1, n2),      // 移除
                   lev_dfs(s1, s2, n1, n2-1),       // 添加
                   lev_dfs(s1, s2, n1-1, n2-1)});   // 替换
}

// 递归，并引入缓存数组
int lev_cache(const string& s1, const string &s2, int n1, int n2, vector<vector<int>>& cache)
{
    if (cache[n1][n2] != -1) return cache[n1][n2];
    if (n1 == 0) return n2;
    if (n2 == 0) return n1;
    if (s1[n1 - 1] == s2[n2 - 1]) {
        cache[n1][n2] = lev_cache(s1, s2, n1-1, n2-1, cache);
        return cache[n1][n2];                                            // 忽略
    }
    cache[n1][n2] =  1 + min({lev_cache(s1, s2, n1-1, n2, cache),        // 移除
                              lev_cache(s1, s2, n1, n2-1, cache),        // 添加
                              lev_cache(s1, s2, n1-1, n2-1, cache)});    // 替换
    return cache[n1][n2];
}

int lev_dfs_cache(const string &s1, const string &s2)
{   
    int n1 = s1.size();
    int n2 = s2.size();
    vector<vector<int>> cache(n1 + 1, vector<int>(n2 + 1, -1));
    return lev_cache(s1, s2, n1-1, n2-1, cache);
}

// 动态规划
int lev_dp(const string &s1, const string &s2)
{
    int n1 = s1.size();
    int n2 = s2.size();
    vector<vector<int>> cache(n1 + 1, vector<int>(n2 + 1, 0));
    for (int i = 1; i <= n2; i++) cache[0][i] = i;
    for (int i = 1; i <= n1; i++) cache[i][0] = i;
    for (int i = 1; i <= n1; i++) {
        for (int j = 1; j <= n2; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                cache[i][j] = cache[i - 1][j - 1];          //忽略              
                continue;
            }
            cache[i][j] =  1 + min({cache[i-1][j],          // 移除
                                      cache[i][j-1],        // 添加
                                      cache[i-1][j-1]});    // 替换 
        }
    }
    return cache[n1][n2];
}

// 操作类型
typedef enum ACTION_TYPE {
    UNKNOWN, IGNORE, ADD, REPLACE, REMOVE
}AT;

// 操作记录
struct OpTuple {
    AT _action;
    char _ch1, _ch2;
    OpTuple(ACTION_TYPE action, char ch) {
        _action = action;
        _ch1 = ch;
        _ch2 = '\0';
    }
    OpTuple(ACTION_TYPE action, char ch1, char ch2) {
        _action = action;
        _ch1 = ch1;
        _ch2 = ch2;
    }
};

// 动态规划，并展示变化过程（eg. horse->rorse->rose->ros）
int lev_dp_action(const string &s1, const string &s2)
{
    int n1 = s1.size();
    int n2 = s2.size();
    vector<vector<int>> cache(n1 + 1, vector<int>(n2 + 1, 0));
    vector<vector<AT>> actions(n1 + 1, vector<AT>(n2 + 1, UNKNOWN));    
    actions[0][0] = IGNORE;
    for (int i = 1; i <= n2; i++) {
        cache[0][i] = i;
        actions[0][i] = ADD;
    }
    for (int i = 1; i <= n1; i++) {
        cache[i][0] = i;
        actions[i][0] = REMOVE;
    }

    for (int i = 1; i <= n1; i++) {
        for (int j = 1; j <= n2; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                cache[i][j] = cache[i - 1][j - 1];
                actions[i][j] = IGNORE;
                continue;
            }

            int replace = cache[i - 1][j - 1];
            int add = cache[i][j - 1];
            int remove = cache[i - 1][j];
            
            cache[i][j] = replace;
            actions[i][j] = REPLACE;
            if (cache[i][j] > add) {
                cache[i][j] = add;
                actions[i][j] = ADD;
            }
            if (cache[i][j] > remove) {
                cache[i][j] = remove;
                actions[i][j] = REMOVE;
            }
            cache[i][j] += 1;
        }
    }
    
    // 记录变化流程
    int idx1 = n1, idx2 = n2;
    vector<OpTuple> trace;
    while (idx1 != 0 || idx2 != 0) {
        AT action = actions[idx1][idx2];
        switch (action)
        {
        case REPLACE: {
                idx1 -= 1;
                idx2 -= 1;
                trace.push_back({REPLACE, s1[idx1], s2[idx2]});
            } break;
        case IGNORE: {
                idx1 -= 1;
                idx2 -= 1;
                trace.push_back({IGNORE, s1[idx1], s2[idx2]});
            } break;
        case ADD: {
                idx2 -= 1;
                trace.push_back({ADD, s2[idx2]});
            } break;
        case REMOVE: {
                idx1 -= 1;
                trace.push_back({REMOVE, s1[idx1]});
            } break;
        default: {
                cout << "Error " << action << endl;
                exit(1);
            }
        }
    }
    reverse(trace.begin(), trace.end());

    // 打印变化流程
    int idx = 0;
    string s3 = s1;
    cout << s1;
    for (int i = 0; i < trace.size(); i++) {
        int action = trace[i]._action;
        switch (action)
        {
        case REPLACE: {
                s3[idx] = trace[i]._ch2;
                idx += 1;
            } break;
        case IGNORE:{
                idx += 1;
                continue;
            }
        case ADD: {
                s3.insert(idx, 1, trace[i]._ch1);
                idx += 1;
            } break;
        case REMOVE: {
                s3.erase(s3.begin() + idx);
            } break;
        default: break;
        }
        cout << "->" << s3;
    }
    cout << endl;
    return cache[n1][n2];
}

int main()
{
    // string s1 = "foo";
    // string s2 = "food";
    // cout << lev_dfs(s1, s2, s1.size(), s2.size()) << endl;

    string s1 = "horse";
    string s2 = "ros";
    cout << "lev_dfs: " << lev_dfs(s1, s2, s1.size()-1, s2.size()-1) << endl;

    // cout << "lev_dfs_cache: " << lev_dfs_cache("foo", "food") << endl;
    cout << "lev_dfs_chace: " << lev_dfs_cache("horse", "ros") << endl;

    cout << "lev_dp: " << lev_dp("horse", "ros") << endl;
    cout << "lev_dp: " << lev_dp("food", "foo") << endl;
    
    cout << "\n字符串转换过程 food → foo: " << endl;
    lev_dp_action("food", "foo");
    cout << "\n字符串转换过程 horse → ros: " << endl;
    lev_dp_action("horse", "ros");
    cout << "\n字符串转换过程 ultramicroscopically → monoultramicroscopicsili: " << endl;
    lev_dp_action("ultramicroscopically", "monoultramicroscopicsili");
}
