#include "mprpcController.h"

MprpcController::MprpcController() :m_failed(false), m_errText("") {}

void MprpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool MprpcController::Failed() const
{
    return m_failed;
}

void MprpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}

std::string MprpcController::ErrorText() const
{
    return m_errText;
}

// 以下是未实现的功能
void MprpcController::StartCancel()
{

}

bool MprpcController::IsCanceled() const
{
    return false;
}

void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback)
{

}


