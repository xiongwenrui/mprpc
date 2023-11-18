#include <iostream>
#include "mprpcApplication.h"
#include "user.pb.h"
#include "mprpcChannel.h"

int main(int argc, char **argv)
{
    // 调用者需要初始化框架后才能使用rpc远程调用
    MprpcApplication::Init(argc, argv);

    // 调用远程发布的rpc方法Login
    MprpcChannel rpcChannel;
    fixbug::UserServiceRpc_Stub userStub(&rpcChannel);

    fixbug::LoginRequest request;
    request.set_name("zzhangsan");
    request.set_pwd("123456");

    fixbug::LoginResponse response;

    userStub.Login(nullptr, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来调用所有rpc方法的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果
    if (0 == response.result().errcode()) {
        std::cout << "rpc login response success:" << response.success() << std::endl;
    } else {
        std::cout << "rpc login response error:" << response.result().errcode() << std::endl;
    }

    fixbug::RegisterRequest registerRequest;
    fixbug::RegisterResponse registerResponse;
    registerRequest.set_id(10086);
    registerRequest.set_name("lisi");
    registerRequest.set_pwd("654321");


    userStub.Register(nullptr, &registerRequest, &registerResponse, nullptr);

    // 一次rpc调用完成，读调用的结果
    if (0 == response.result().errcode()) {
        std::cout << "rpc login response success:" << response.success() << std::endl;
    } else {
        std::cout << "rpc login response error:" << response.result().errcode() << std::endl;
    }

    return 0;
}