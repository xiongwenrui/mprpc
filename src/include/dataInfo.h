#pragma once

#include "google/protobuf/service.h"
#include <unordered_map>

// 服务类型信息
struct ServiceInfo
{
    google::protobuf::Service *m_service; // 保存服务对象
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存服务方法
};