#include "mprpcChannel.h"
#include "rpcHeader.pb.h"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mprpcApplication.h>
#include <netinet/in.h>
#include <unistd.h>
#include "zookeeperutil.h"

/*
header_size + service_name method_name args_size + args
*/
// 所有stub代理对象调用rpc方法的入口
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, 
                          google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (!request->SerializeToString(&args_str)) {
        controller->SetFailed("serialize args error!");
        return;
    } else {
        args_size = args_str.size();
        std::cout << "test args_str:" << args_str << " args_size:" << args_size << std::endl;
    }

    // mprpc header封装
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string header_str;
    if (!rpcHeader.SerializeToString(&header_str)) {
        controller->SetFailed("rpcHeader serialize error!");
        return;
    } else {
        header_size = header_str.size();
    }

    // 准备发送rpc接口调用请求
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    std::cout << "send_rpc_str:" << send_rpc_str << std::endl;
    send_rpc_str += header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "===============================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "header_str:" << header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_size:" << args_size << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "===============================" << std::endl;

    // 使用TCP编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        char errText[512] = {0};
        sprintf(errText, "create socket error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }

    // 读取配置文件rpcserver信息
    // std::string ip = MprpcApplication::GetInstance()->GetMprpcConfig().Load("rpcServerIP");
    // uint16_t port = atoi(MprpcApplication::GetInstance()->GetMprpcConfig().Load("rpcServerPort").c_str());

    // 从zk上获取服务信息
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());

    if (host_data == "") {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }

    int index = host_data.find(":");

    if (index == host_data.npos) {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }

    std::string ip = host_data.substr(0, index);
    int port = atoi(host_data.substr(index + 1).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct  sockaddr*)&server_addr, sizeof(server_addr)))
    {
        char errText[512] = {0};
        sprintf(errText, "connnet error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    };

    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        char errText[512] = {0};
        sprintf(errText, "send error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        char errText[512] = {0};
        sprintf(errText, "recv error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        
        return;
    }

    // 反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, 0, recv_size);
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        char errText[512] = {0};
        sprintf(errText, "parse error! recv_buf:%s", recv_buf);
        controller->SetFailed(errText);
        close(clientfd);

        return;
    }

    close(clientfd);
}
