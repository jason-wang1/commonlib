#pragma once
#include <string>
#include <unordered_map>
#include <any>
#include "Common/RateLimiter.h"
#include "Common/DynamicThreadPool.h"
#include "GrpcDispatcher/AsyncDefine.h"
#include "GrpcDispatcher/AsyncReceiver.h"

using OnGrpcFunc = std::function<
    bool(std::any obj,
         const int cmd,
         const long deadline_ms,
         const std::string &request,
         int &result,
         std::string &response)>;

// 命令类型信息
class CmdInfo
{
public:
    int32_t cmd;
    std::any obj; // 传递的都是this指针, 故不存在值拷贝问题
    OnGrpcFunc func;
    RateLimiter *p_rate_limiter; // 接口限速器
};

// 本类仅在注册时初始化命令信息列表, 无需加锁
class GrpcDispatcher
{
public:
    static GrpcDispatcher *GetInstance()
    {
        static GrpcDispatcher instance_;
        return &instance_;
    }

    // 绑定命令信息
    void Bind(
        std::any obj,
        const int cmd,
        OnGrpcFunc func,
        int64_t limit_qps = 0,
        int64_t limit_cache = 0)
    {
        CmdInfo &info = cmd_info_list_[cmd];
        info.cmd = cmd;
        info.obj = obj;
        info.func = func;

        // 传0表示不限制QPS
        if (limit_qps > 0)
        {
            info.p_rate_limiter = new RateLimiter(limit_qps, limit_cache);
        }
    }

    // Grpc请求分发
    bool Dispatch(const int cmd, const long deadline_ms, const std::string &request,
                  int &result, std::string &response) const
    {
        auto iter = cmd_info_list_.find(cmd);
        if (iter == cmd_info_list_.end())
        {
            result = GrpcProtos::ResultType::ERR_Service_CMD;
            response = "cmd not found";
            return false;
        }

        const CmdInfo &info = iter->second;
        if (info.p_rate_limiter != nullptr)
        {
            if (!info.p_rate_limiter->try_pass(3))
            {
                result = GrpcProtos::ResultType::ERR_Rate_Limit;
                response = "rate limit";
                return false;
            }
        }
        return info.func(info.obj, info.cmd, deadline_ms, request, result, response);
    }

    // grpc 异步请求分发
    bool AsyncDispatch(
        const int cmd,
        const long deadline_ms,
        const std::string &request,
        AsyncReceiver *receiver)
    {
        auto iter = cmd_info_list_.find(cmd);
        if (iter == cmd_info_list_.end())
        {
            int result = GrpcProtos::ResultType::ERR_Service_CMD;
            std::string response = "cmd not found";

            ResponseProto resp_proto;
            resp_proto.set_cmd(cmd);
            resp_proto.set_result(result);
            resp_proto.set_response(std::move(response));
            receiver->Response(resp_proto);
            return false;
        }

        const CmdInfo &info = iter->second;
        if (info.p_rate_limiter != nullptr)
        {
            if (!info.p_rate_limiter->try_pass(3))
            {
                int result = GrpcProtos::ResultType::ERR_Rate_Limit;
                std::string response = "rate limit";

                ResponseProto resp_proto;
                resp_proto.set_cmd(cmd);
                resp_proto.set_result(result);
                resp_proto.set_response(std::move(response));
                receiver->Response(resp_proto);
                return false;
            }
        }

        auto proc_func = [](const CmdInfo &info,
                            const long deadline_ms,
                            const std::string &request,
                            AsyncReceiver *receiver)
        {
            int result = GrpcProtos::ResultType::ERR_Unknown;
            std::string response;
            info.func(info.obj, info.cmd, deadline_ms, request, result, response);

            ResponseProto resp_proto;
            resp_proto.set_cmd(info.cmd);
            resp_proto.set_result(result);
            resp_proto.set_response(std::move(response));
            receiver->Response(resp_proto);
        };
        DynamicThreadPool::GetInstance()->Add(proc_func, info, deadline_ms, request, receiver);
        return true;
    }

private:
    std::unordered_map<int32_t, CmdInfo> cmd_info_list_; // 命令信息列表
};
