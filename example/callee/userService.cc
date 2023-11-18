#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcApplication.h"
#include "rpcProvider.h"
#include "friendService.h"
#include "logger.h"

/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和Register
*/
class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;

        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << " name:" << name << " pwd:" << pwd << std::endl;

        return true;
    }

    /*  重写基类UserServideRpc的虚函数，下面函数都是框架直接调用
        1. caller ===> Login(LoginRequest) => muduo -> callee
        2. callee ===> Login(LoginRequest) => 交给下面重写的这个函数
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest,业务获取响应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入
        fixbug::ResultCode *resultCode = response->mutable_result();
        resultCode->set_errcode(0);
        resultCode->set_errmsg("");
        response->set_success(login_result);

        // 执行回调函数 执行响应对象数据的序列化和网络发送（由框架完成）
        done->Run();
    }

    void Register(google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(id, name, pwd);

        fixbug::ResultCode *resultCode = response->mutable_result();
        resultCode->set_errcode(0);
        resultCode->set_errmsg("");
        response->set_success(register_result);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}