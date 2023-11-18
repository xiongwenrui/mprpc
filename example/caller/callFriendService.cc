#include <iostream>
#include "mprpcApplication.h"
#include "friend.pb.h"
#include "mprpcChannel.h"
#include "mprpcController.h"

int main(int argc, char **argv)
{
    // 调用者需要初始化框架后才能使用rpc远程调用
    MprpcApplication::Init(argc, argv);

    // 调用远程发布的rpc方法Login
    MprpcChannel rpcChannel;
    fixbug::FriendServiceRpc_Stub friendStub(&rpcChannel);

    fixbug::GetFriendListRequest request;
    request.set_userid(1);

    fixbug::GetFriendListResponse response;

    MprpcController mprpcController;
    friendStub.GetFriendsList(&mprpcController, &request, &response, nullptr);

    // 一次调用完成，查看调用结果
    if (!mprpcController.Failed()) {
        std::cout << "rpc login response success!" << std::endl;
        std::cout << "response friends size:" << response.friends_size() << std::endl;
        for (int i = 0; i < response.friends_size(); i++) {
            std::cout << "index:" << i << " friend:" << response.friends(i) << std::endl;
        }
    } else {
        std::cout << mprpcController.ErrorText() << std::endl;
    }

    return 0;
}