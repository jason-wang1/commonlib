#pragma once
#include <vector>
#include <thread>
#include <future>
#include <unordered_map>
#include "ClientManager/GrpcSender.h"
#include "ClientManager/GrpcClient.h"

#include "Common/SafeQueue.h"

class UnifiedClient final
    : public GrpcClient<GrpcSender>
{
public:
    UnifiedClient() {}
    virtual ~UnifiedClient() {}

    void Init(const std::string &helloRequest,
              const GrpcProtos::ServiceType serviceType,
              const std::string &relySemver);

    void ShutDown();

public:
    void PushRequestTime(const double &requestTime);

    void HandleRequestTime();

    double GetAvgRequestTime();

    bool CallRemoteServer(
        const int cmd,
        const std::string &loadBalanceElement,
        const std::string &request,
        const long tiemout,
        int &result,
        std::string &response);

    bool UpdateByAddr(const GrpcProtos::ServiceInfo &serviceInfo);

    void DoHello(const std::vector<std::string> &addrList);

    bool OpenByAddr(const std::string &addr);

    bool CloseByAddr(const std::string &addr);

    void GetWatchServiceInfo(GrpcProtos::WatchServiceInfo *watchServiceInfo);

private:
    // Hello请求信息
    std::string m_HelloRequest;

    // 服务类型
    GrpcProtos::ServiceType m_ServiceType;

    // 依赖版本号
    std::string m_RelySemver;

    // 服务列表
    std::unordered_map<std::string, GrpcProtos::ServiceInfo> m_AddrToServiceMap;

    // 服务列表读写锁
    std::shared_mutex m_AddrToServiceMapRWMutex;

    // Hello状态
    std::unordered_map<std::string, std::atomic<bool>> m_AddrToHelloStatusMap;

    // Hello状态读写锁
    std::shared_mutex m_AddrToHelloStatusMapRWMutex;

    bool m_Terminate = false;

    // 下级服务请求时间
    SafeQueue<double> m_RequestTimeQueue;
    std::shared_ptr<std::thread> m_RequestTimeThreadPtr;

    // 双缓冲
    std::atomic<int> m_dataIdx = 0;
    double m_dBufCache[2] = {0.0};
};