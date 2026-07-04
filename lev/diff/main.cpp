#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <functional>
#include <unordered_map>
using namespace std;

// diff操作类型
typedef enum ACTION_TYPE {
    UNKNOWN, IGNORE, ADD, REPLACE, REMOVE
}AT;

// diff操作记录
struct OpTuple {
    AT _action;
    string _ch1, _ch2;
    OpTuple(ACTION_TYPE action, string ch) {
        _action = action;
        _ch1 = ch;
        _ch2 = '\0';
    }
    OpTuple(ACTION_TYPE action, string ch1, string ch2) {
        _action = action;
        _ch1 = ch1;
        _ch2 = ch2;
    }
};

// 莱文斯坦距离
int lev(const string &s1, const string &s2)
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

    return cache[n1][n2];
}

// 基于莱文斯坦距离diff
int lev_diff(const vector<string> &lines1, const vector<string> &lines2, vector<OpTuple> &trace)
{
    int n1 = lines1.size();
    int n2 = lines2.size();
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
            if (lines1[i - 1] == lines2[j - 1]) {
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
    while (idx1 != 0 || idx2 != 0) {
        AT action = actions[idx1][idx2];
        switch (action)
        {
        case REPLACE: {
                idx1 -= 1;
                idx2 -= 1;
                trace.push_back({REPLACE, lines1[idx1], lines2[idx2]});
            } break;
        case IGNORE: {
                idx1 -= 1;
                idx2 -= 1;
                trace.push_back({IGNORE, lines1[idx1], lines2[idx2]});
            } break;
        case ADD: {
                idx2 -= 1;
                trace.push_back({ADD, lines2[idx2]});
            } break;
        case REMOVE: {
                idx1 -= 1;
                trace.push_back({REMOVE, lines1[idx1]});
            } break;
        default: {
                cout << "Error " << action << endl;
                exit(1);
            }
        }
    }
    reverse(trace.begin(), trace.end());

    return cache[n1][n2];
}

// ------------------------------------------------------

// 读文件
vector<string> read_file(const string &file_path)
{
    ifstream file(file_path);
    vector<string> lines;
    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    return lines; // C++11后会使用move,开销与传出参数差不多
}

// 系统命令
void diff_command(int argc, char *argv[])
{
    if (argc < 2) {
        cout << "diff: \"" << argv[argc - 1] << "\" 后缺少操作对象" << endl;
        cout << "diff: 请尝试执行 \"help\" 来获取更多信息。" << endl;
        exit(1);
    }
    if (argc > 2) {
        cout << "diff: 多余操作对象 \"" << argv[2] << "\"" << endl;
        cout << "diff: 请尝试执行 \"help\" 来获取更多信息。" << endl;
        exit(1);
    }
    
    string file_path1 = argv[0];
    string file_path2 = argv[1];
    vector<string> lines1(read_file(file_path1));
    vector<string> lines2(read_file(file_path2));

    vector<OpTuple> trace;
    lev_diff(lines1, lines2, trace);
    
    cout << "--- " << file_path1 << endl;
    cout << "+++ " << file_path2 << endl;
    for (auto item : trace) {
        AT action = item._action;
        switch (action) {
            case ADD : {
                cout << "+" << item._ch1 << endl;
            } break;
            case REMOVE : {
                cout << "-" << item._ch1 << endl;
            } break;
            case REPLACE : {
                cout << "-" << item._ch1 << endl;
                cout << "+" << item._ch2 << endl;
            } break;
            default : break;
        }
    }
}

void test_command(int argc, char *argv[])
{
    // TODO
}

void help_command(int argc, char *argv[]);

// 系统命令函数表
typedef function<void(int, char*[])> func;
struct command {
    func _function;
    string _signature;
    string _description;

    command() = default;

    command(func function, string signature, string description) {
        _function = function;
        _signature = signature;
        _description = description;
    }
};

unordered_map<string, command> command_map = {
    {"help", {help_command, "help [subcommand]", "打印命令的相关信息"}},
    {"diff", {diff_command, "diff <file1> <file2>", "把文件之间的差异打印到标准输出"}},
    // {"test1",  {test_command, "null", "null"}},
    // {"test22",  {test_command, "null", "null"}},
    // {"test333",  {test_command, "null", "null"}},
};

void help_command(int argc, char *argv[])
{
    static int width1 = 0;   // 用于输出对齐
    static int width2 = 0;
    if (width1 == 0) {
        for (auto &kv : command_map) {
            width1 = max(width1,
                         int(kv.first.size()));
            width2 = max(width2, 
                         int(kv.second._signature.size()));
        }
        width1 += 1;
        width2 += 1;
    }

    if (argc > 1) {
        cout << "help: 多余操作对象 \"" << argv[1] << "\"" << endl;
        exit(1);
    }

    for (const auto& kv : command_map)
    {
        const string &name = kv.first;
        const command &info = kv.second;

        cout << name << string(width1 - name.size(), ' ');
        cout << info._signature << string(width2 - info._signature.size(), ' ');
        cout << info._description << endl;
    }
}

// ------------------------------------------------------

void suggest_command(string command_name)
{
    vector<pair<int, string>> candidates;
    for (const auto& kv : command_map) {
        const string &name = kv.first;
        int distance = lev(command_name, name);
        if (distance < 3) {
            candidates.push_back({distance, name});
        }
    }
    sort(candidates.begin(), candidates.end());
    if (candidates.size() == 0) {
        cout << command_name << ": 未找到命令" << endl;
        return;
    }
    cout << "找不到命令 \"" << command_name << "\" 您的意思可能是: \"";
    cout << candidates[0].second << "\"" << endl;
}

void parse_command(int argc, char *argv[]) 
{
    if (argc == 1) {
        cout << "缺少操作对象";
        exit(1);
    }

    string command_name = argv[1];
    if (command_map.find(command_name) == command_map.end()) {
        suggest_command(command_name);
    }
    else {
        command_map[command_name]._function(argc - 2, argv + 2);
    }
}

int main(int argc, char *argv[]) 
{
    // cout << "argc: " << argc << endl;
    // for (int i = 0; i < argc; i++) {
    //     cout << argv[i] << endl;
    // }
    parse_command(argc, argv);

    return 0;
}