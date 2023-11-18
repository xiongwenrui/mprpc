#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <iostream>

// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // zkserver根据path创建znode结点
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数制定的znode结点路径获取znode结点的值
    std::string GetData(const char *path);

private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};