#pragma once

#include <google/protobuf/service.h>

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    void SetFailed(const std::string& reason);
    std::string ErrorText() const;

    // 未实现
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed; // RPC方法执行过程中的状态
    std::string m_errText; // RPC方法执行过程中的错误信息
};