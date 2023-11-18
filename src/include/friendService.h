#pragma once

#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcApplication.h"
#include "rpcProvider.h"
#include <vector>

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid);

    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done);

};