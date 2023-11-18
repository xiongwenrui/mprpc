#include <iostream>
#include "test.pb.h"
#include <string>

using namespace fixbug;

int main() 
{
    LoginResponse rsp;
    ResultCode *rc1 = rsp.mutable_result();
    rc1->set_errcode(1);
    rc1->set_errmsg("登录处理失败了");
    std::string rsp_str;

    if (rsp.SerializeToString(&rsp_str)) {
        std::cout << "响应体序列化成功：" << std::endl;
        std::cout << "errcode: " << rc1->errcode() << " errmsg: " << rc1->errmsg() << std::endl;
    }

    GetFriendListsResponse grsp;
    ResultCode *rc2 = grsp.mutable_result();
    rc2->set_errcode(0);
    rc2->set_errmsg("");

    User *user1 = grsp.add_friend_list();
    user1->set_name("zhangsan");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    std::cout << grsp.friend_list_size() << std::endl;

    User *user2 = grsp.add_friend_list();
    user2->set_name("lisi");
    user2->set_age(18);
    user2->set_sex(User::WOMAN);

    std::cout << grsp.friend_list_size() << std::endl;
}

int main1() {
    // 封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    // 数据对象的序列化 =》 char*
    std::string send_str;
    if (req.SerializeToString(&send_str)) {
        std::cout << send_str.c_str() << std::endl;
    }

    // 从sent_ser反序列化一个login请求对象
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str)) {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}