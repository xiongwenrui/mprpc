#include "friendService.h"

std::vector<std::string> FriendService::GetFriendsList(uint32_t userid)
{
    std::cout << "do GetFriendsList service! userid:" << userid << std::endl;
    std::vector<std::string> friendsList;

    friendsList.push_back("yangrenzheng");
    friendsList.push_back("ouyulin");
    friendsList.push_back("tanzhengfu");
    friendsList.push_back("chenpeien");
    friendsList.push_back("zhengxinglong");

    return friendsList;
}

void FriendService::GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
{
    uint32_t id = request->userid();

    std::vector<std::string> friendsVec = GetFriendsList(id);

    fixbug::FriendResultCode *resultCode = response->mutable_result();
    resultCode->set_errcode(0);
    resultCode->set_errmsg("");

    google::protobuf::RepeatedPtrField<std::string> *friends = response->mutable_friends();

    for (std::string &f : friendsVec) {
        friends->Add()->assign(f);
    }

    done->Run();
}