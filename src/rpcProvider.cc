#include "rpcProvider.h"
#include "mprpcApplication.h"
#include <string>
#include "rpcHeader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

/*
service_name => service描述
                => service 记录服务对象
                    method_name => 方法对象
*/

// 存储注册成功的服务对象和其服务方法的所有信息
static std::unordered_map<std::string, ServiceInfo> m_serviceInfoMap;

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo serviceInfo;

    // 获取服务描述信息
    const google::protobuf::ServiceDescriptor *pServiceDescriptor = service->GetDescriptor();
    // 获取服务名字
    std::string serviceName = pServiceDescriptor->name();

    // 获取服务service方法的数量
    int serviceMethodNumber = pServiceDescriptor->method_count();

    LOG_INFO("service_name:%s", serviceName.c_str());

    /*
        1.serviceInfo记录每个服务地址和服务方法集合的映射
        2.m_serviceMap记录每个服务名字和服务信息的映射
    */
    for (int i = 0; i < serviceMethodNumber; i++) {
        // 获取服务对象制定下标的服务方法的描述
        const google::protobuf::MethodDescriptor *pMethodDescripot = pServiceDescriptor->method(i);
        std::string methodName = pMethodDescripot->name();
        serviceInfo.m_methodMap.insert({methodName, pMethodDescripot});
        LOG_INFO("method_name:%s", methodName.c_str());
    }
    
    serviceInfo.m_service = service;

    m_serviceInfoMap.insert({serviceName, serviceInfo});
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected()) {
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好通信用的protobuf数据类型
service_name method_name args 定义proto的message类型，进行数据的序列化和反序列化
                        service_name method_name args_size
UserServiceLogin zhangsan 123456

header_size(4字节) + header_set() + args_str
*/
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                            muduo::net::Buffer* buffer, muduo::Timestamp)
{
    // 网络上接收的远程rpc调用请求的字节流 Login args
    std::string recv_buf = buffer->retrieveAllAsString();
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字节流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name = "";
    std::string method_name = "";
    uint32_t args_size = 0;
    if (rpcHeader.ParseFromString(rpc_header_str)) {
        // 解析成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } else {
        // 解析失败
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }
    
    // 解析args参数
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "=====================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_size:" << args_size << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "=====================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceInfoMap.find(service_name);
    if (it == m_serviceInfoMap.end()) {
        std::cout << service_name << " is not exits!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end()) {
        std::cout << service_name << ":" << method_name << " is not exits!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service; // 获取service对象
    const google::protobuf::MethodDescriptor *methodDescriptor = mit->second; // 获取一个method指针

    // 生成rpc方法调用请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(methodDescriptor).New();

    if(!request->ParseFromString(args_str)) {
        std::cout << service_name << ":" << method_name << " request args parse error!" << std::endl;
        return;
    }

    google::protobuf::Message *response = service->GetResponsePrototype(methodDescriptor).New();

    // 给下面method方法绑定一个Closure回调函数
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>
                                                        (this, &RpcProvider::SendRpcResponse, conn, response);

    // 根据远端请求调用本地方法
    service->CallMethod(methodDescriptor, nullptr, request, response, done);
}

void RpcProvider::Run()
{
    std::string rpcServerIP = MprpcApplication::GetInstance()->GetMprpcConfig().Load("rpcServerIP");
    uint16_t rpcServerPort = atoi(MprpcApplication::GetInstance()->GetMprpcConfig().Load("rpcServerPort").c_str());

    muduo::net::InetAddress address(rpcServerIP, rpcServerPort);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定连接回调和消息读写回调函数   分离网络代码和业务代码
    // bind函数绑定普通函数时，会将函数名隐式转换为函数指针；但绑定成员函数时，必须显示地制定函数指针，即需要加&
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
                              std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库的线程数量
    server.setThreadNum(2);

    ZkClient zkCli;
    zkCli.Start();

    // service_name 为永久性结点 method_name为临时性节点
    for (auto &sp : m_serviceInfoMap) {
        // service_name
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for  (auto &mp : sp.second.m_methodMap) {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", rpcServerIP.c_str(), rpcServerPort);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    std::cout << "RpcProvider start service at ip:" << rpcServerIP << " port:" << rpcServerPort << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        // 序列化成功
        conn->send(response_str);
    } else {
        // 序列化失败
        std::cout << "response serialize error!" << std::endl;
    }

    conn->shutdown(); // 模拟http短连接，rpcprovider主动断开
}