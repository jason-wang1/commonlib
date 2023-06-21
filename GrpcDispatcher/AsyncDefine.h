#pragma once
#include "Protobuf/proxy/Common.grpc.pb.h"

enum AsyncStatus
{
    CREATE,
    PROCESS,
    FINISH
};

using RequestProto = GrpcProtos::UnifiedRequest;
using ResponseProto = GrpcProtos::UnifiedResponse;

using AsyncService = GrpcProtos::UnifiedService::AsyncService;
using AsyncResponseWriter = grpc::ServerAsyncResponseWriter<ResponseProto>;
