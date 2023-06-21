#include <string>
#include <memory>
#include <any>

#include "Common/Function.h"
#include "grpcpp/grpcpp.h"
#include "grpc/support/log.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"

namespace TDPredict
{
    using Service = tensorflow::serving::PredictionService;
    using RequestProto = tensorflow::serving::PredictRequest;
    using ResponseProto = tensorflow::serving::PredictResponse;

    using InputMap = google::protobuf::Map<std::string, tensorflow::TensorProto>;
    using OutputMap = google::protobuf::Map<std::string, tensorflow::TensorProto>;

    using GrpcCallBackFunc = std::function<bool(
        const grpc::Status status,
        ResponseProto &response,
        std::any data)>;

    class TFservingClient
    {
    public:
        // addr = ipv4:address[:port][,address[:port],...]
        // see: https://github.com/grpc/grpc/blob/master/doc/naming.md
        explicit TFservingClient(const std::vector<std::string> &vec_addr)
        {
            std::string addr = "ipv4:";
            addr.reserve(vec_addr.size() * 20);
            for (auto &item : vec_addr)
            {
                addr.append(item);
                addr.append(",");
            }
            addr = Common::trim(addr, ',');

            grpc::ChannelArguments args;
            args.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
            args.SetLoadBalancingPolicyName("round_robin");
            stub_ = Service::NewStub(grpc::CreateCustomChannel(addr, grpc::InsecureChannelCredentials(), args));
        }

        // 异步请求接口, 当请求完成后调用回调函数
        void AsyncSend(
            RequestProto &request,
            const long timeout_ms,
            GrpcCallBackFunc func,
            std::any data) const
        {
            AsyncClientCall *call = new AsyncClientCall;
            call->data = data;
            call->func = func;

            if (timeout_ms > 0)
            {
                gpr_timespec timespec;
                timespec.tv_sec = timeout_ms / 1000;
                timespec.tv_nsec = (timeout_ms % 1000) * 1000 * 1000;
                timespec.clock_type = GPR_TIMESPAN;
                call->context.set_deadline(timespec);
            }

            call->response_reader = stub_->PrepareAsyncPredict(&call->context, request, &cq_);
            call->response_reader->StartCall();
            call->response_reader->Finish(&call->response, &call->status, (void *)call);
        }

        void PipeSend(
            grpc::CompletionQueue *cq,
            RequestProto &request,
            const long timeout_ms,
            GrpcCallBackFunc func,
            std::any data) const
        {
            AsyncClientCall *call = new AsyncClientCall;
            call->data = data;
            call->func = func;

            if (timeout_ms > 0)
            {
                gpr_timespec timespec;
                timespec.tv_sec = timeout_ms / 1000;
                timespec.tv_nsec = (timeout_ms % 1000) * 1000 * 1000;
                timespec.clock_type = GPR_TIMESPAN;
                call->context.set_deadline(timespec);
            }

            call->response_reader = stub_->AsyncPredict(&call->context, request, cq);
            call->response_reader->Finish(&call->response, &call->status, (void *)call);
        }

        bool PipeRecv(grpc::CompletionQueue *cq) const
        {
            // P.s> 需要注意这里不保证Pipe的顺序
            // 故实际的方案是, 调用几次Send, 就调用对应次数的Recv
            // 比如调用Send顺序 [1,2,3,4], 处理Recv顺序可能是 [2,3,1,4]
            // 这种情况下需要自行知道返回结果的顺序
            if (cq == nullptr)
            {
                return false;
            }

            bool ret = false;
            {
                void *got_tag;
                bool ok = false;
                GPR_ASSERT(cq->Next(&got_tag, &ok));
                GPR_ASSERT(ok);
                AsyncClientCall *call = static_cast<AsyncClientCall *>(got_tag);
                ret = call->func(call->status, call->response, call->data);
                delete call;
            }
            return ret;
        }

        void AsyncCompleteRpc()
        {
            void *got_tag;
            bool ok = false;
            while (cq_.Next(&got_tag, &ok))
            {
                GPR_ASSERT(ok);
                AsyncClientCall *call = static_cast<AsyncClientCall *>(got_tag);
                call->func(call->status, call->response, call->data);
                delete call;
            }
        }

        // 同步接口, 请求结束后调用func
        bool Send(
            RequestProto &request,
            const long timeout_ms,
            GrpcCallBackFunc func,
            std::any data) const
        {
            grpc::ClientContext context;
            if (timeout_ms > 0)
            {
                gpr_timespec timespec;
                timespec.tv_sec = timeout_ms / 1000;
                timespec.tv_nsec = (timeout_ms % 1000) * 1000 * 1000;
                timespec.clock_type = GPR_TIMESPAN;
                context.set_deadline(timespec);
            }

            ResponseProto response;
            auto status = stub_->Predict(&context, request, &response);
            return func(status, response, data);
        }

    private:
        struct AsyncClientCall
        {
            grpc::ClientContext context;
            std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseProto>> response_reader;
            grpc::Status status;
            ResponseProto response;
            GrpcCallBackFunc func;
            std::any data;
        };

        std::unique_ptr<Service::Stub> stub_;
        mutable grpc::CompletionQueue cq_;
    };
}