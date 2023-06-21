#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <any>
#include "Common/Singleton.h"
#include "ClientManager/GrpcSender.h"
#include "ClientManager/GrpcClient.h"

class UnifiedClient;

class RegisterCenter
    : public GrpcClient<GrpcSender>,
      public Singleton<RegisterCenter>
{
public:
    RegisterCenter(token) {}
    virtual ~RegisterCenter() {}
    RegisterCenter(RegisterCenter &) = delete;
    RegisterCenter &operator=(const RegisterCenter &) = delete;

public:
    struct ServiceConfig
    {
        // 当前服务信息
        GrpcProtos::ServiceInfo serviceInfo;

        // Hello Request 内容
        std::string helloRequest;

        // 是否上报ping
        bool isSendPing;
    };

public:
    bool Init(
        const std::vector<std::string> &regCenterAddrList,
        const GrpcProtos::ServiceInfo &serviceInfo);

    bool Online();

    bool Offline();

    bool addByAddr(const ServiceConfig &svcConf);

    bool OnlineByAddr(const std::string &auxAddr);

    bool OfflineByAddr(const std::string &auxAddr);

    void ShutDown();

public:
    static bool OnNotify(std::any obj, const int cmd, const long deadline, const std::string &request, int &result, std::string &response);
    static bool OnHello(std::any obj, const int cmd, const long deadline, const std::string &request, int &result, std::string &response);

public:
    double GetAvgRequestTime(
        const GrpcProtos::ServiceType watchServiceType);

    bool CallRemoteServer(
        const GrpcProtos::ServiceType watchServiceType,
        const int cmd,
        const std::string &loadBalanceElement,
        const std::string &request,
        const long timeout,
        int &result,
        std::string &response);

private:
    bool DoRegister(const ServiceConfig &svcConf);

    bool DoOnline(const ServiceConfig &svcConf);

    bool DoPing();

    bool DoCheck();

    bool DoOffline(const ServiceConfig &svcConf);

    std::string GetRegCenterAddr();

    UnifiedClient *GetClientByType(GrpcProtos::ServiceType watchServiceType) const;

private:
    static std::atomic<bool> g_timer;
    void OnTimer();

private:
    // 服务管理地址
    std::vector<std::string> m_RegCenterAddrList;

    // 主服务 地址
    std::string m_OwnerAddr;

    // 主服务 Hello Request 内容
    std::string m_OwnerHelloRequest;

    // 服务地址对应 服务配置
    std::unordered_map<std::string, ServiceConfig> m_AddrServiceConfigList;

    // 服务地址对应 服务配置 读写锁
    std::shared_mutex m_AddrServiceConfigRWMutex;

    // 主服务 关注的下级服务类型对应的客户端列表
    std::unordered_map<GrpcProtos::ServiceType, std::shared_ptr<UnifiedClient>> m_OwnerWatchTypeClientList;

    // to do 6: 新增管理辅助服务下级服务信息结构

private:
    static long m_LastCheckTime;

    // 1000毫秒报送一次心跳
    static long m_PingInterval;

    // 10000毫秒检查一次关注服务，保证完整性
    static long m_CheckInterval;
};
