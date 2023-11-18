#include "mprpcApplication.h"
#include <iostream>
#include <unistd.h>

void ShowArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv)
{
    if (argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1) {
        switch (c)
        {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // 开始加载配置文件 rpcserver_ip= rpcserver_port zookeeper_ip zookepper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcServerIP:" << m_config.Load("rpcServerIP") << std::endl;
    // std::cout << "rpcServerPort:" << m_config.Load("rpcServerPort") << std::endl;

    // std::cout << "zookeeperIP:" << m_config.Load("zookeeperIP") << std::endl;
    // std::cout << "zookeeperPort:" << m_config.Load("zookeeperPort") << std::endl;
}

MprpcConfig &MprpcApplication::GetMprpcConfig()
{
    return m_config;
}

MprpcApplication *MprpcApplication::GetInstance()
{
    if (MprpcApplication::mprpcApp == nullptr) {
        MprpcApplication::mprpcApp = new MprpcApplication();
    }

    return mprpcApp;
}

MprpcApplication *MprpcApplication::mprpcApp = GetInstance();
MprpcConfig MprpcApplication::m_config;