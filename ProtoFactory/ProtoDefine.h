#pragma once
#include <memory>
#include <google/protobuf/message.h>

struct ProtoDeleter
{
    void operator()(google::protobuf::Message *pMessage) const
    {
        if (pMessage != nullptr)
        {
            delete pMessage;
            pMessage = nullptr;
        }
    }
};

using SharedPtrProto = std::shared_ptr<google::protobuf::Message>;
using UniquePtrProto = std::unique_ptr<google::protobuf::Message, ProtoDeleter>;
