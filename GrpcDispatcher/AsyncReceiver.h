#pragma once
#include <string>
#include <grpcpp/grpcpp.h>
#include "GrpcDispatcher/AsyncDefine.h"

class AsyncReceiver
{
public:
    AsyncReceiver(std::shared_ptr<AsyncService> asyncService,
                  grpc::ServerCompletionQueue *completionQueue)
        : m_spService(asyncService),
          m_pCompletionQueue(completionQueue),
          m_ResponseWriter(&m_ctx)
    {
        m_status = AsyncStatus::CREATE;
        Proceed();
    }

    void Proceed();

    void Response(const ResponseProto &resp_proto);

private:
    AsyncStatus m_status;
    grpc::ServerContext m_ctx;
    RequestProto request;

private:
    std::shared_ptr<AsyncService> m_spService = nullptr;
    grpc::ServerCompletionQueue *m_pCompletionQueue = nullptr;
    AsyncResponseWriter m_ResponseWriter;
};