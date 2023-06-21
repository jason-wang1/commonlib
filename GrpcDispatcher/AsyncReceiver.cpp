#include "AsyncReceiver.h"
#include "GrpcDispatcher.h"

void AsyncReceiver::Proceed()
{
    if (m_status == CREATE)
    {
        m_status = PROCESS;
        m_spService->RequestCallService(
            &m_ctx, &request,
            &m_ResponseWriter,
            m_pCompletionQueue,
            m_pCompletionQueue, this);
    }
    else if (m_status == PROCESS)
    {
        new AsyncReceiver(m_spService, m_pCompletionQueue);

        long deadline_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                               m_ctx.deadline().time_since_epoch())
                               .count();
        // 异步分发接口
        GrpcDispatcher::GetInstance()->AsyncDispatch(request.cmd(), deadline_ms, request.request(), this);
    }
    else
    {
        GPR_ASSERT(m_status == FINISH);
        delete this;
    }
}

void AsyncReceiver::Response(const ResponseProto &msg)
{
    m_status = AsyncStatus::FINISH;
    m_ResponseWriter.Finish(msg, grpc::Status::OK, this);
}