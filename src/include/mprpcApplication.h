#pragma once

#include "mprpcConfig.h"

// mprpc框架的初始化类
class MprpcApplication
{
public:
    static MprpcApplication *GetInstance();
    static MprpcConfig &GetMprpcConfig();
    static void Init(int argc, char **argv);
    
private:
    MprpcApplication() {};
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;

    ~MprpcApplication() {
        delete mprpcApp;
    }

    static MprpcConfig m_config;
    static MprpcApplication *mprpcApp;
};