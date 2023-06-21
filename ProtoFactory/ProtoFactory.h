#pragma once
#include "ProtoDefine.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

class ProtoFactory final
{
public:
    ProtoFactory()
        : m_stdd(&m_st),
          m_des_pool(&m_stdd),
          m_dynamic_factory(&m_des_pool) {}

    ProtoFactory(const std::string &proto_path)
        : m_stdd(&m_st),
          m_des_pool(&m_stdd),
          m_dynamic_factory(&m_des_pool)
    {
        Init(proto_path);
    }

    bool Init(const std::string &proto_path);

    SharedPtrProto GetProtoMessage(const std::string &proto_name) noexcept;

private:
    google::protobuf::compiler::DiskSourceTree m_st;
    google::protobuf::compiler::SourceTreeDescriptorDatabase m_stdd;
    google::protobuf::DescriptorPool m_des_pool;
    google::protobuf::DynamicMessageFactory m_dynamic_factory;
};
