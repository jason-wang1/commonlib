#pragma once
#include <string>
#include <grpcpp/grpcpp.h>
#include "AsyncDefine.h"
#include "GrpcDispatcher.h"

// Grpc接收器类
class GrpcReceiver final : public GrpcProtos::UnifiedService::Service
{
public:
    // 实现调用服务逻辑, Grpc请求分发
    grpc::Status CallService(
        grpc::ServerContext *context,
        const GrpcProtos::UnifiedRequest *request,
        GrpcProtos::UnifiedResponse *response) override
    {
        long deadline_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                               context->deadline().time_since_epoch())
                               .count();
        response->set_cmd(request->cmd());
        std::string &resp = *response->mutable_response();

        int result = GrpcProtos::ERR_Unknown;
        GrpcDispatcher::GetInstance()->Dispatch(request->cmd(), deadline_ms, request->request(), result, resp);

        response->set_result(result);

        return grpc::Status::OK;
    }
};