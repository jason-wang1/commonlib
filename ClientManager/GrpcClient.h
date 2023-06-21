#pragma once
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include "grpcpp/grpcpp.h"
#include "Protobuf/proxy/Common.grpc.pb.h"
#include "MurmurHash3/MurmurHash3.h"

// Grpc客户端类
template <typename T>
class GrpcClient
{
public:
    GrpcClient() {}
    virtual ~GrpcClient() {}

    // 打开客户端
    bool OpenClient(const std::string &addr, const int service_weight, const int connect_mode)
    {
        T *client = GetClientByAddr(addr);
        if (client != nullptr)
        {
            if (!updateSwitch(addr, service_weight, connect_mode, true))
            {
                AddClient(addr, service_weight, connect_mode);
            }
        }
        else
        {
            // 新增客户端Grpc默认打开
            AddClient(addr, service_weight, connect_mode);
        }

        return true;
    }

    // 关闭客户端
    bool CloseClient(const std::string &addr, const int service_weight, const int connect_mode)
    {
        T *client = GetClientByAddr(addr);
        if (client != nullptr)
        {
            updateSwitch(addr, service_weight, connect_mode, false);
        }

        return true;
    }

    bool Send(const std::string &addr, const int cmd,
              const std::string &request, const long timeout_ms,
              int &result, std::string &response)
    {
        T *client = GetClientByAddr(addr);
        if (client == nullptr)
        {
            result = GrpcProtos::ResultType::ERR_NO_Server;
            response = "No online service";
            return false;
        }

        if (client->CallService(cmd, request, timeout_ms, result, response))
        {
            return true;
        }

        return false;
    }

    bool Send(const int cmd, const std::string &loadBalanceElement,
              const std::string &request, const long timeout_ms,
              std::string &addr, int &result, std::string &response)
    {
        T *client = GetRandClient(loadBalanceElement);
        if (client == nullptr)
        {
            result = GrpcProtos::ResultType::ERR_NO_Server;
            response = "No online service";
            return false;
        }

        addr = client->addr_;

        if (client->CallService(cmd, request, timeout_ms, result, response))
        {
            return true;
        }

        return false;
    }

private:
    bool AddClient(const std::string &addr, const int service_weight, const int connect_mode)
    {
        auto client = std::shared_ptr<T>(new T());
        client->service_weight_ = service_weight;
        client->connect_mode_ = connect_mode;

        grpc::ChannelArguments args;
        args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
        args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 1000);
        args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
        args.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 1);
        args.SetInt(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 5000);
        args.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
        auto channel = grpc::CreateCustomChannel(addr, grpc::InsecureChannelCredentials(), args);
        client->Init(addr, channel);

        // 写锁, 出作用域自动释放
        std::unique_lock<std::shared_mutex> ul(client_list_rwmutex_);
        client_list_[addr] = client;

        return true;
    }

    T *GetClientByAddr(const std::string &addr)
    {
        // 读锁, 出作用域自动释放
        std::shared_lock<std::shared_mutex> sl(client_list_rwmutex_);

        auto iter = client_list_.find(addr);
        if (iter != client_list_.end())
        {
            return iter->second.get();
        }

        return nullptr;
    }

    T *GetRandClient(const std::string &loadBalanceElement)
    {
        // 读锁, 出作用域自动释放
        std::shared_lock<std::shared_mutex> sl(client_list_rwmutex_);

        if (client_list_.empty())
        {
            return nullptr;
        }

        int count = 0;
        std::unordered_map<int, T *> weightClient;
        for (auto iter = client_list_.begin(); iter != client_list_.end(); ++iter)
        {
            T *client = iter->second.get();
            if (!client->grpc_switch_)
            {
                continue;
            }

            // 临时兼容逻辑，如果权重为0，则默认权重32
            int weight = client->service_weight_;
            if (weight <= 0)
            {
                weight = 32;
            }

            for (int i = 0; i < weight; i++)
            {
                weightClient[count++] = client;
            }
        }

        if (weightClient.empty())
        {
            return nullptr;
        }

        unsigned int loadBalanceValue = 0;
        if (loadBalanceElement.empty())
        {
            loadBalanceValue = rand();
        }
        else
        {
            MurmurHash3_x86_32(loadBalanceElement.c_str(), loadBalanceElement.length(), 31, &loadBalanceValue);
        }

        return weightClient[loadBalanceValue % weightClient.size()];
    }

    void GetOnlineClient(std::vector<T *> onlineList)
    {
        // 读锁, 出作用域自动释放
        std::shared_lock<std::shared_mutex> sl(client_list_rwmutex_);

        if (!client_list_.empty())
        {
            onlineList.reserve(client_list_.size());
            for (auto iter = client_list_.begin(); iter != client_list_.end(); ++iter)
            {
                if (iter->second.get()->grpc_switch_)
                {
                    onlineList.push_back(iter->second.get());
                }
            }
        }
    }

    bool updateSwitch(const std::string &addr, const int service_weight, const int connect_mode, bool status)
    {
        // 写锁, 出作用域自动释放
        std::unique_lock<std::shared_mutex> ul(client_list_rwmutex_);

        auto iter = client_list_.find(addr);
        if (iter != client_list_.end())
        {
            iter->second.get()->grpc_switch_ = status;
            iter->second.get()->service_weight_ = service_weight;
            if (iter->second.get()->connect_mode_ != connect_mode)
            {
                return false;
            }
        }

        return true;
    }

private:
    // 写时读取可能会有Rehash问题导致迭代器失效,
    // 读写锁只保证map读取&写入时安全
    std::shared_mutex client_list_rwmutex_;

    // 客户端列表
    std::unordered_map<std::string, std::shared_ptr<T>> client_list_;
};
