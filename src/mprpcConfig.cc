#include "mprpcConfig.h"
#include <iostream>

void trim(std::string &s, char c)
{
    int index = 0;
    if (!s.empty()) {
        while ((index = s.find(c, index)) != std::string::npos) {
            s.erase(index, 1);
        }
    }
}

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf) {
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 1.注释 2.正确的配置项 3.去掉开头多余的空格
    // feof()判断指针是否指向文件结束，false表示未指向文件结束，true表示指向文件结束
    while(!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // 去掉字符串前面多余的空格
        std::string src_buf(buf);
        trim(src_buf, ' ');

        // // 在字符串中寻找第一个不为空格的字符，若寻找不到，则返回-1
        // int idx = src_buf.find_first_not_of(' ');
        // if (idx != -1)
        // {
        //     // 字符串前面可能有空格
        //     src_buf = src_buf.substr(idx, src_buf.size() - idx);
        // }
        // // 去掉字符串后面多余的空格
        // idx = src_buf.find_last_not_of(' ');
        // if (idx != -1) {
        //     // 字符串后面可能有空格
        //     src_buf = src_buf.substr(0, idx + 1);
        // }



        // 判断#注释和空格行
        if (src_buf[0] == '#' || src_buf.empty()) {
            continue;
        }

        // 解析配置项
        int idx = src_buf.find('=');
        if (idx == std::string::npos) {
            // 配置项不合法
            continue;
        }

        std::string key;
        std::string value;
        key = src_buf.substr(0, idx);

        trim(src_buf, '\n');
        trim(src_buf, '\r');
        value = src_buf.substr(idx + 1);

        unMap.insert(std::pair<std::string, std::string>(key, value));
    }
}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    std::unordered_map<std::string, std::string>::iterator it = unMap.find(key);

    if (it != unMap.end()) {
        return it->second;
    }

    return "";
}