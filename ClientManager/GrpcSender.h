#pragma once
#include <string>
#include "grpcpp/grpcpp.h"
#include "Protobuf/proxy/Common.grpc.pb.h"

// Grpc发送器类
class GrpcSender
{
public:
    GrpcSender() {}
    ~GrpcSender() {}

    // 初始化Grpc存根，支持设置毫秒级连接超时
    void Init(const std::string &addr, std::shared_ptr<grpc::Channel> channel)
    {
        addr_ = addr;
        stub_ = GrpcProtos::UnifiedService::NewStub(channel);

        grpc_switch_ = true;
    }

    // 调用远程服务
    bool CallService(
        const int cmd,
        const std::string &request,
        long timeout_ms,
        int &result,
        std::string &response)
    {
        // 判定为不可能出现的情况
        if (!grpc_switch_)
        {
            result = GrpcProtos::ResultType::ERR_Grpc_Closed;
            response = "grpc switch closed";
            return false;
        }

        GrpcProtos::UnifiedRequest unifiedRequest;
        unifiedRequest.set_cmd(cmd);
        unifiedRequest.set_request(request);

        grpc::ClientContext context;
        if (timeout_ms > 0)
        {
            gpr_timespec timespec;
            timespec.tv_sec = timeout_ms / 1000;
            timespec.tv_nsec = (timeout_ms % 1000) * 1000 * 1000;
            timespec.clock_type = GPR_TIMESPAN;
            context.set_deadline(timespec);
        }

        GrpcProtos::UnifiedResponse unifiedResponse;
        auto stubRet = stub_->CallService(&context, unifiedRequest, &unifiedResponse);
        if (!stubRet.ok())
        {
            switch (stubRet.error_code())
            {
            case grpc::StatusCode::DEADLINE_EXCEEDED:
            {
                result = GrpcProtos::ResultType::ERR_Service_Timeout;
                break;
            }
            default:
            {
                result = GrpcProtos::ResultType::ERR_Call_Service;
                break;
            }
            }
            response = stubRet.error_message();

            return false;
        }

        result = unifiedResponse.result();
        response = unifiedResponse.response();
        return true;
    }

public:
    std::string addr_;   // 发送器地址
    bool grpc_switch_;   // grpc开关
    int service_weight_; // 服务权重
    int connect_mode_;   // 连接模式

private:
    // 远程服务的本地存根
    std::unique_ptr<GrpcProtos::UnifiedService::Stub> stub_;
};