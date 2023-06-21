// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: proxy/Common.proto

#include "proxy/Common.pb.h"
#include "proxy/Common.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace GrpcProtos {

static const char* UnifiedService_method_names[] = {
  "/GrpcProtos.UnifiedService/CallService",
};

std::unique_ptr< UnifiedService::Stub> UnifiedService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< UnifiedService::Stub> stub(new UnifiedService::Stub(channel));
  return stub;
}

UnifiedService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_CallService_(UnifiedService_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status UnifiedService::Stub::CallService(::grpc::ClientContext* context, const ::GrpcProtos::UnifiedRequest& request, ::GrpcProtos::UnifiedResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::GrpcProtos::UnifiedRequest, ::GrpcProtos::UnifiedResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_CallService_, context, request, response);
}

void UnifiedService::Stub::experimental_async::CallService(::grpc::ClientContext* context, const ::GrpcProtos::UnifiedRequest* request, ::GrpcProtos::UnifiedResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::GrpcProtos::UnifiedRequest, ::GrpcProtos::UnifiedResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CallService_, context, request, response, std::move(f));
}

void UnifiedService::Stub::experimental_async::CallService(::grpc::ClientContext* context, const ::GrpcProtos::UnifiedRequest* request, ::GrpcProtos::UnifiedResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CallService_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::GrpcProtos::UnifiedResponse>* UnifiedService::Stub::PrepareAsyncCallServiceRaw(::grpc::ClientContext* context, const ::GrpcProtos::UnifiedRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::GrpcProtos::UnifiedResponse, ::GrpcProtos::UnifiedRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_CallService_, context, request);
}

::grpc::ClientAsyncResponseReader< ::GrpcProtos::UnifiedResponse>* UnifiedService::Stub::AsyncCallServiceRaw(::grpc::ClientContext* context, const ::GrpcProtos::UnifiedRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncCallServiceRaw(context, request, cq);
  result->StartCall();
  return result;
}

UnifiedService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      UnifiedService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< UnifiedService::Service, ::GrpcProtos::UnifiedRequest, ::GrpcProtos::UnifiedResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](UnifiedService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::GrpcProtos::UnifiedRequest* req,
             ::GrpcProtos::UnifiedResponse* resp) {
               return service->CallService(ctx, req, resp);
             }, this)));
}

UnifiedService::Service::~Service() {
}

::grpc::Status UnifiedService::Service::CallService(::grpc::ServerContext* context, const ::GrpcProtos::UnifiedRequest* request, ::GrpcProtos::UnifiedResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace GrpcProtos
