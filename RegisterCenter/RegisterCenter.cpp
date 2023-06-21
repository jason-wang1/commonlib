#include "RegisterCenter.h"
#include <thread>
#include "glog/logging.h"
#include "timer/timer.h"
#include "semver/semver.hpp"
#include "Common/Function.h"
#include "GrpcDispatcher/GrpcDispatcher.h"
#include "UnifiedClient.h"

long RegisterCenter::m_LastCheckTime = 0;
long RegisterCenter::m_PingInterval = 3000;
long RegisterCenter::m_CheckInterval = 30000;

std::atomic<bool> RegisterCenter::g_timer(false);

bool RegisterCenter::Init(
    const std::vector<std::string> &regCenterAddrList,
    const GrpcProtos::ServiceInfo &serviceInfo)
{
    m_RegCenterAddrList = regCenterAddrList;
    for (std::string &regCenterAddr : m_RegCenterAddrList)
    {
        OpenClient(regCenterAddr, 0, 0);
    }

    const std::string &ownerAddr = serviceInfo.addr();
    if (ownerAddr.empty())
    {
        LOG(ERROR) << "RegisterCenter::Init(): Owner Addr is empty failed!";
        return false;
    }

    m_OwnerAddr = ownerAddr;

    std::stringstream helloRequest;
    helloRequest << "The hello is " << serviceInfo.service_type();
    helloRequest << " from " << serviceInfo.addr();
    m_OwnerHelloRequest = helloRequest.str();

    auto &ownerSvcConf = m_AddrServiceConfigList[m_OwnerAddr];
    ownerSvcConf.serviceInfo.CopyFrom(serviceInfo);

    ownerSvcConf.helloRequest = m_OwnerHelloRequest;
    ownerSvcConf.serviceInfo.set_status(GrpcProtos::ServiceStatus::Register);
    ownerSvcConf.isSendPing = true;

    // 获取处理器数量
    if (ownerSvcConf.serviceInfo.service_weight() <= 0)
    {
        ownerSvcConf.serviceInfo.set_service_weight(sysconf(_SC_NPROCESSORS_ONLN));
    }

    for (const auto &relyInfo : ownerSvcConf.serviceInfo.rely_list())
    {
        GrpcProtos::ServiceType relyServiceType = static_cast<GrpcProtos::ServiceType>(relyInfo.rely_service_type());

        std::shared_ptr<UnifiedClient> clientPtr = std::shared_ptr<UnifiedClient>(new UnifiedClient);
        clientPtr->Init(m_OwnerHelloRequest, relyServiceType, relyInfo.rely_semver());
        m_OwnerWatchTypeClientList.insert(std::pair(relyServiceType, clientPtr));
    }

    GrpcDispatcher::GetInstance()->Bind(this, GrpcProtos::CMD_NOTIFY, OnNotify, 0, 0);
    GrpcDispatcher::GetInstance()->Bind(this, GrpcProtos::CMD_HELLO, OnHello, 0, 0);

    if (!DoRegister(ownerSvcConf))
    {
        LOG(ERROR) << "RegisterCenter::Init(): DoRegister failed!";
        return false;
    }

    // 拉起线程定时调用更新函数
    g_timer = true;
    auto funcTimerFunc = std::bind(&RegisterCenter::OnTimer, this);
    std::thread(funcTimerFunc).detach();

    return true;
}

bool RegisterCenter::Online()
{
    auto iter = m_AddrServiceConfigList.find(m_OwnerAddr);
    if (iter != m_AddrServiceConfigList.end())
    {
        iter->second.serviceInfo.set_status(GrpcProtos::ServiceStatus::Online);
        if (!DoOnline(iter->second))
        {
            LOG(ERROR) << "RegisterCenter::Online(): DoOnline failed!";
            return false;
        }
    }

    return true;
}

bool RegisterCenter::Offline()
{
    // 所有客户端下线
    for (auto &item : m_AddrServiceConfigList)
    {
        if (item.second.serviceInfo.status() == GrpcProtos::ServiceStatus::Online)
        {
            item.second.serviceInfo.set_status(GrpcProtos::ServiceStatus::Offline);
            DoOffline(item.second);
        }
    }

    return true;
}

bool RegisterCenter::addByAddr(const ServiceConfig &svcConf)
{
    const std::string &auxAddr = svcConf.serviceInfo.addr();
    if (auxAddr == m_OwnerAddr)
    {
        LOG(ERROR) << "RegisterCenter::addByAddr(): No Support Owner Addr!";
        return false;
    }

    if (auxAddr.empty())
    {
        LOG(ERROR) << "RegisterCenter::Init(): aux Addr is empty failed!";
        return false;
    }

    std::unique_lock<std::shared_mutex> ul(m_AddrServiceConfigRWMutex);
    auto iter = m_AddrServiceConfigList.find(auxAddr);
    if (iter != m_AddrServiceConfigList.end())
    {
        iter->second.serviceInfo.set_status(GrpcProtos::ServiceStatus::Register);
        if (!DoRegister(iter->second))
        {
            LOG(ERROR) << "RegisterCenter::addByAddr(): DoRegister failed!";
            return false;
        }

        iter->second.isSendPing = true;
    }
    else
    {
        auto &newSvcConf = m_AddrServiceConfigList[auxAddr];
        newSvcConf = svcConf;

        std::stringstream helloRequest;
        helloRequest << "The hello is " << newSvcConf.serviceInfo.service_type();
        helloRequest << " from " << newSvcConf.serviceInfo.addr();

        newSvcConf.helloRequest = helloRequest.str();
        newSvcConf.serviceInfo.set_status(GrpcProtos::ServiceStatus::Register);

        if (!DoRegister(newSvcConf))
        {
            LOG(ERROR) << "RegisterCenter::addByAddr(): DoRegister failed!";
            return false;
        }

        newSvcConf.isSendPing = true;
    }

    return true;
}

bool RegisterCenter::OnlineByAddr(const std::string &auxAddr)
{
    if (auxAddr == m_OwnerAddr)
    {
        LOG(ERROR) << "RegisterCenter::OnlineByAddr(): No Support Owner Addr!";
        return false;
    }

    std::unique_lock<std::shared_mutex> ul(m_AddrServiceConfigRWMutex);
    auto iter = m_AddrServiceConfigList.find(auxAddr);
    if (iter != m_AddrServiceConfigList.end())
    {
        iter->second.serviceInfo.set_status(GrpcProtos::ServiceStatus::Online);
        if (!DoOnline(iter->second))
        {
            LOG(ERROR) << "RegisterCenter::OnlineByAddr(): DoOnline failed!";
            return false;
        }

        iter->second.isSendPing = true;
    }

    return true;
}

bool RegisterCenter::OfflineByAddr(const std::string &auxAddr)
{
    if (auxAddr == m_OwnerAddr)
    {
        LOG(ERROR) << "RegisterCenter::OnlineByAddr(): No Support Owner Addr!";
        return false;
    }

    std::unique_lock<std::shared_mutex> ul(m_AddrServiceConfigRWMutex);
    auto iter = m_AddrServiceConfigList.find(auxAddr);
    if (iter != m_AddrServiceConfigList.end())
    {
        iter->second.isSendPing = false;
        iter->second.serviceInfo.set_status(GrpcProtos::ServiceStatus::Offline);
        if (!DoOffline(iter->second))
        {
            LOG(ERROR) << "RegisterCenter::OfflineByAddr(): DoOffline failed!";
            return false;
        }
    }

    return true;
}

void RegisterCenter::ShutDown()
{
    // 停止定时任务
    g_timer = false;

    // 所有客户端下线
    Offline();

    for (auto &item : m_OwnerWatchTypeClientList)
    {
        item.second->ShutDown();
    }
}

bool RegisterCenter::OnNotify(std::any obj, const int cmd, const long deadline, const std::string &request, int &result, std::string &response)
{
    GrpcProtos::NotifyRequest protoRequest;
    if (!protoRequest.ParseFromString(request))
    {
        result = GrpcProtos::ResultType::ERR_Decode_Request;
        response = "Parse Notify Request error";
    }

    const ::GrpcProtos::ServiceInfo &serviceInfo = protoRequest.service_info();

    std::string relyService = "";
    for (int idx = 0; idx < serviceInfo.rely_list_size(); idx++)
    {
        if (idx == 0)
        {
            relyService = std::to_string(serviceInfo.rely_list(idx).rely_service_type());
        }
        else
        {
            relyService += ", " + std::to_string(serviceInfo.rely_list(idx).rely_service_type());
        }
        relyService += ":" + serviceInfo.rely_list(idx).rely_semver();
    }

    LOG(INFO) << "OnNotify() service_type = " << serviceInfo.service_type()
              << ", semver = " << serviceInfo.semver()
              << ", addr = " << serviceInfo.addr()
              << ", status = " << serviceInfo.status()
              << ", rely_list = " << relyService
              << ", deadline = " << deadline;

    GrpcProtos::ServiceType watchServiceType = static_cast<GrpcProtos::ServiceType>(serviceInfo.service_type());

    if (obj.type() != typeid(RegisterCenter *))
    {
        LOG(ERROR) << "OnNotify() obj.type() != typeid(RegisterCenter *)";
        result = GrpcProtos::ResultType::ERR_Service_Cal;
        response = "ClientManager obj type error.";
        return false;
    }
    RegisterCenter *regCenter = std::any_cast<RegisterCenter *>(obj);
    UnifiedClient *clientPtr = regCenter->GetClientByType(watchServiceType);
    if (clientPtr != nullptr)
    {
        clientPtr->UpdateByAddr(serviceInfo);
    }

    result = GrpcProtos::ResultType::OK;
    response = "ok";
    return true;
}

bool RegisterCenter::OnHello(std::any obj, const int cmd, const long deadline, const std::string &request, int &result, std::string &response)
{
    LOG(INFO) << "OnHello() request = " << request << ", deadline = " << deadline;

    result = GrpcProtos::ResultType::OK;
    response = "ok";

    return true;
}

double RegisterCenter::GetAvgRequestTime(
    const GrpcProtos::ServiceType watchServiceType)
{
    return GetClientByType(watchServiceType)->GetAvgRequestTime();
}

bool RegisterCenter::CallRemoteServer(
    const GrpcProtos::ServiceType watchServiceType,
    const int cmd,
    const std::string &loadBalanceElement,
    const std::string &request,
    const long timeout,
    int &result,
    std::string &response)
{
    UnifiedClient *clientPtr = GetClientByType(watchServiceType);
    if (clientPtr == nullptr)
    {
        // 下级服务不存在, 可能是未关注该类型服务
        result = GrpcProtos::ERR_Service_CMD;
        response = "Service type not rely";
        return false;
    }

    if (!clientPtr->CallRemoteServer(cmd, loadBalanceElement, request, timeout, result, response))
    {
        return false;
    }

    return true;
}

bool RegisterCenter::DoRegister(const ServiceConfig &svcConf)
{
    GrpcProtos::RegisterRequest requestProto;
    requestProto.mutable_service_info()->CopyFrom(svcConf.serviceInfo);

    std::string request = "";
    if (!requestProto.SerializeToString(&request))
    {
        LOG(ERROR) << "RegisterCenter::DoRegister(): Request proto serialize to string failed!";
        return false;
    }

    std::string regCenterAddr = GetRegCenterAddr();
    if (regCenterAddr.empty())
    {
        LOG(ERROR) << "RegisterCenter::DoRegister(): Register Center Addr is empty!";
        return false;
    }

    int result = 0;
    std::string response = "";
    if (!Send(regCenterAddr, GrpcProtos::CMD_REGISTER, request, 1000L, result, response))
    {
        LOG(ERROR) << "RegisterCenter::DoRegister(): To register center register failed"
                   << ", regCenterAddr = " << regCenterAddr;
        return false;
    }

    if (result != GrpcProtos::ResultType::OK)
    {
        LOG(ERROR) << "RegisterCenter::DoRegister(): To register center register failed"
                   << ", regCenterAddr = " << regCenterAddr
                   << ", result = " << result
                   << ", response = " << response;
        return false;
    }

    GrpcProtos::RegisterReply protoReply;
    if (!protoReply.ParseFromString(response))
    {
        LOG(ERROR) << "RegisterCenter::DoRegister(): Reply proto data parse failed!";
        return false;
    }

    if (svcConf.serviceInfo.addr() == m_OwnerAddr)
    {
        for (int listIdx = 0; listIdx < protoReply.watch_list_size(); listIdx++)
        {
            const auto &watchServiceInfo = protoReply.watch_list(listIdx);
            GrpcProtos::ServiceType watchServiceType = static_cast<GrpcProtos::ServiceType>(watchServiceInfo.service_type());

            UnifiedClient *clientPtr = GetClientByType(watchServiceType);
            if (clientPtr == nullptr)
            {
                continue;
            }

            for (int infoIdx = 0; infoIdx < watchServiceInfo.service_list_size(); infoIdx++)
            {
                const GrpcProtos::ServiceInfo &serviceInfo = watchServiceInfo.service_list(infoIdx);
                clientPtr->UpdateByAddr(serviceInfo);
            }
        }
    }

    return true;
}

bool RegisterCenter::DoOnline(const ServiceConfig &svcConf)
{
    GrpcProtos::RegisterRequest requestProto;
    requestProto.mutable_service_info()->CopyFrom(svcConf.serviceInfo);

    std::string request = "";
    if (!requestProto.SerializeToString(&request))
    {
        LOG(ERROR) << "RegisterCenter::DoOnline(): Request proto serialize to string failed!";
        return false;
    }

    std::string regCenterAddr = GetRegCenterAddr();
    if (regCenterAddr.empty())
    {
        LOG(ERROR) << "RegisterCenter::DoOnline(): Register Center Addr is empty!";
        return false;
    }

    int result = 0;
    std::string response = "";
    if (!Send(regCenterAddr, GrpcProtos::CMD_ONLINE, request, 1000L, result, response))
    {
        LOG(ERROR) << "RegisterCenter::DoOnline(): To register center online failed"
                   << ", regCenterAddr = " << regCenterAddr;
        return false;
    }

    if (result != GrpcProtos::ResultType::OK)
    {
        LOG(ERROR) << "RegisterCenter::DoOnline(): To register center online failed"
                   << ", regCenterAddr = " << regCenterAddr
                   << ", result = " << result
                   << ", response = " << response;
        return false;
    }

    GrpcProtos::RegisterReply protoReply;
    if (!protoReply.ParseFromString(response))
    {
        LOG(ERROR) << "RegisterCenter::DoOnline(): Reply proto data parse failed!";
        return false;
    }

    if (svcConf.serviceInfo.addr() == m_OwnerAddr)
    {
        for (int listIdx = 0; listIdx < protoReply.watch_list_size(); listIdx++)
        {
            const auto &watchServiceInfo = protoReply.watch_list(listIdx);
            GrpcProtos::ServiceType watchServiceType = static_cast<GrpcProtos::ServiceType>(watchServiceInfo.service_type());

            UnifiedClient *clientPtr = GetClientByType(watchServiceType);
            if (clientPtr == nullptr)
            {
                continue;
            }

            for (int infoIdx = 0; infoIdx < watchServiceInfo.service_list_size(); infoIdx++)
            {
                const GrpcProtos::ServiceInfo &serviceInfo = watchServiceInfo.service_list(infoIdx);
                clientPtr->UpdateByAddr(serviceInfo);
            }
        }
    }

    return true;
}

bool RegisterCenter::DoPing()
{
    for (auto &item : m_AddrServiceConfigList)
    {
        if (item.second.isSendPing)
        {
            GrpcProtos::PingRequest requestProto;
            requestProto.mutable_service_info()->CopyFrom(item.second.serviceInfo);

            std::string request = "";
            if (!requestProto.SerializeToString(&request))
            {
                return false;
            }

            std::string regCenterAddr = GetRegCenterAddr();
            if (regCenterAddr.empty())
            {
                LOG(ERROR) << "RegisterCenter::DoPing(): Register Center Addr is empty!";
                return false;
            }

            int result = 0;
            std::string response = "";
            if (!Send(regCenterAddr, GrpcProtos::CMD_PING, request, 1000L, result, response))
            {
                LOG(ERROR) << "RegisterCenter::DoPing(): To register center ping failed"
                           << ", regCenterAddr = " << regCenterAddr;
                return false;
            }

            if (response != "ok")
            {
                LOG(ERROR) << "RegisterCenter::DoPing(): To register center ping failed!";
            }

            if (result != GrpcProtos::ResultType::OK || response != "ok")
            {
                LOG(ERROR) << "RegisterCenter::DoPing(): To register center ping failed!"
                           << ", regCenterAddr = " << regCenterAddr
                           << ", result = " << result
                           << ", response = " << response;
                return false;
            }
        }
    }

    return true;
}

bool RegisterCenter::DoCheck()
{
    auto iter = m_AddrServiceConfigList.find(m_OwnerAddr);
    if (iter != m_AddrServiceConfigList.end())
    {
        GrpcProtos::CheckRequest requestProto;
        requestProto.mutable_service_info()->CopyFrom(iter->second.serviceInfo);

        for (auto clientIter = m_OwnerWatchTypeClientList.begin(); clientIter != m_OwnerWatchTypeClientList.end(); ++clientIter)
        {
            clientIter->second->GetWatchServiceInfo(requestProto.add_watch_list());
        }

        std::string request = "";
        if (!requestProto.SerializeToString(&request))
        {
            return false;
        }

        std::string regCenterAddr = GetRegCenterAddr();
        if (regCenterAddr.empty())
        {
            LOG(ERROR) << "RegisterCenter::DoCheck(): Register Center Addr is empty!";
            return false;
        }

        int result = 0;
        std::string response = "";
        if (!Send(regCenterAddr, GrpcProtos::CMD_CHECK, request, 1000L, result, response))
        {
            LOG(ERROR) << "RegisterCenter::DoCheck(): To register center check failed"
                       << ", regCenterAddr = " << regCenterAddr;
            return false;
        }

        if (result != GrpcProtos::ResultType::OK)
        {
            LOG(ERROR) << "RegisterCenter::DoCheck(): To register center check failed"
                       << ", regCenterAddr = " << regCenterAddr
                       << ", result = " << result
                       << ", response = " << response;
            return false;
        }

        if (response != "ok")
        {
            LOG(WARNING) << "RegisterCenter::DoCheck(): To register center check failed!";

            GrpcProtos::CheckReply protoReply;
            if (!protoReply.ParseFromString(response))
            {
                LOG(ERROR) << "RegisterCenter::DoCheck(): Reply proto data parse failed!";
                return false;
            }

            for (int listIdx = 0; listIdx < protoReply.watch_list_size(); listIdx++)
            {
                const auto &watchServiceInfo = protoReply.watch_list(listIdx);
                GrpcProtos::ServiceType watchServiceType = static_cast<GrpcProtos::ServiceType>(watchServiceInfo.service_type());

                UnifiedClient *clientPtr = GetClientByType(watchServiceType);
                if (clientPtr == nullptr)
                {
                    continue;
                }

                for (int infoIdx = 0; infoIdx < watchServiceInfo.service_list_size(); infoIdx++)
                {
                    const GrpcProtos::ServiceInfo &serviceInfo = watchServiceInfo.service_list(infoIdx);
                    clientPtr->UpdateByAddr(serviceInfo);
                }
            }
        }
    }

    return true;
}

bool RegisterCenter::DoOffline(const ServiceConfig &svcConf)
{
    GrpcProtos::OfflineRequest requestProto;
    requestProto.mutable_service_info()->CopyFrom(svcConf.serviceInfo);

    std::string request = "";
    if (!requestProto.SerializeToString(&request))
    {
        return false;
    }

    std::string regCenterAddr = GetRegCenterAddr();
    if (regCenterAddr.empty())
    {
        LOG(ERROR) << "RegisterCenter::DoOffline(): Register Center Addr is empty!";
        return false;
    }

    int result = 0;
    std::string response = "";
    if (!Send(regCenterAddr, GrpcProtos::CMD_OFFLINE, request, 1000L, result, response))
    {
        LOG(ERROR) << "RegisterCenter::DoOffline(): To register center offline failed"
                   << ", regCenterAddr = " << regCenterAddr;
        return false;
    }

    if (response != "ok")
    {
        LOG(ERROR) << "RegisterCenter::DoOffline(): To register center offline failed!";
    }

    if (result != GrpcProtos::ResultType::OK || response != "ok")
    {
        LOG(ERROR) << "RegisterCenter::DoOffline(): To register center offline failed!"
                   << ", regCenterAddr = " << regCenterAddr
                   << ", result = " << result
                   << ", response = " << response;
        return false;
    }

    return true;
}

std::string RegisterCenter::GetRegCenterAddr()
{
    std::string regCenterAddr = m_RegCenterAddrList.at(rand() % m_RegCenterAddrList.size());

    int result = 0;
    std::string response = "";
    if (Send(regCenterAddr, GrpcProtos::CMD_HELLO, m_OwnerHelloRequest, 1000L, result, response))
    {
        if (result == GrpcProtos::ResultType::OK && response == "ok")
        {
            return regCenterAddr;
        }
    }

    for (std::string &availableRegCenterAddr : m_RegCenterAddrList)
    {
        if (Send(availableRegCenterAddr, GrpcProtos::CMD_HELLO, m_OwnerHelloRequest, 1000L, result, response))
        {
            if (result == GrpcProtos::ResultType::OK && response == "ok")
            {
                return availableRegCenterAddr;
            }
        }
    }

    return "";
}

UnifiedClient *RegisterCenter::GetClientByType(GrpcProtos::ServiceType watchServiceType) const
{
    const auto iter = m_OwnerWatchTypeClientList.find(watchServiceType);
    if (iter != m_OwnerWatchTypeClientList.end())
    {
        return iter->second.get();
    }
    return nullptr;
}

void RegisterCenter::OnTimer()
{
    while (g_timer)
    {
        // 每秒操作一次
        std::this_thread::sleep_for(std::chrono::milliseconds(m_PingInterval));

        if (!g_timer)
        {
            break;
        }

        DoPing();

        long timeStamp = Common::get_ms_timestamp();
        if (timeStamp - m_LastCheckTime > m_CheckInterval)
        {
            m_LastCheckTime = timeStamp;
            DoCheck();
        }
    }
}
