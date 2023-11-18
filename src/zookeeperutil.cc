#include "zookeeperutil.h"
#include "mprpcApplication.h"

ZkClient::ZkClient() :m_zhandle(nullptr) {}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    /*
        zookeeper_mt: 多线程版本
        zookeeper的API客户端程序提供了三个线程
        API调用线程
        网络I/O线程 pthread_create poll
        wathcer回调线程
    */
    std::string host = MprpcApplication::GetInstance()->GetMprpcConfig().Load("zookeeperIP");
    std::string port = MprpcApplication::GetInstance()->GetMprpcConfig().Load("zookeeperPort");
    std::string connstr = host + ":" + port;

    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    
    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}

// zkserver根据path创建znode结点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buf[128];
    int buf_len = sizeof(path_buf);
    int flag;
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)
    {
        flag = zoo_create(m_zhandle, path, data, datalen,
                &ZOO_OPEN_ACL_UNSAFE, state, path_buf, buf_len);
        
        if (flag == ZOK)
        {
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数制定的znode结点路径获取znode结点的值
std::string ZkClient::GetData(const char *path)
{
    char buf[64];
    int buf_len = sizeof(buf);
    int flag = zoo_get(m_zhandle, path, 0, buf, &buf_len, nullptr);
    if (flag != ZOK)
    {
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }
    else {
        return buf;
    }

}