#include "UnifiedClient.h"
#include <thread>
#include "glog/logging.h"
#include "semver/semver.hpp"
#include "Common/Function.h"

void UnifiedClient::Init(
    const std::string &helloRequest,
    const GrpcProtos::ServiceType serviceType,
    const std::string &relySemver)
{
    m_HelloRequest = helloRequest;
    m_ServiceType = serviceType;
    m_RelySemver = relySemver;

    m_Terminate = true;
    m_RequestTimeThreadPtr = std::make_shared<std::thread>(std::bind(&UnifiedClient::HandleRequestTime, this));
}

void UnifiedClient::PushRequestTime(const double &requestTime)
{
    m_RequestTimeQueue.push_back(requestTime);
}

void UnifiedClient::HandleRequestTime()
{
    size_t batchSize = 100;

    double sumTime = 0.0;
    size_t sumCount = 0;

    int cacheCount = 0;
    while (m_Terminate)
    {
        std::vector<double> vecRequestTime;
        vecRequestTime.reserve(batchSize);
        if (this->m_RequestTimeQueue.pop_front(vecRequestTime, batchSize, 10, m_Terminate))
        {
            for (double &requestTime : vecRequestTime)
            {
                sumTime += requestTime;
                sumCount++;
            }
        }

        cacheCount++;
        if (cacheCount >= 10)
        {
            double avgRequestTime = sumTime / sumCount;

            if (sumCount > 0)
            {
                LOG(INFO) << "UnifiedClient::HandleRequestTime() Statis Avg Time"
                          << ", sumTime = " << sumTime
                          << ", sumCount = " << sumCount
                          << ", avgRequestTime = " << avgRequestTime;
            }

            int newDataIdx = (m_dataIdx + 1) % 2;
            m_dBufCache[newDataIdx] = avgRequestTime;
            m_dataIdx = newDataIdx;

            cacheCount = 0;
            sumTime = 0.0;
            sumCount = 0;
        }
    }
}

double UnifiedClient::GetAvgRequestTime()
{
    return m_dBufCache[m_dataIdx];
}

void UnifiedClient::ShutDown()
{
    if (m_Terminate)
    {
        // 停止定时任务
        m_Terminate = false;

        if (m_RequestTimeThreadPtr != nullptr)
        {
            if (m_RequestTimeThreadPtr->joinable())
            {
                m_RequestTimeThreadPtr->join();
            }
        }
    }
}

bool UnifiedClient::CallRemoteServer(
    const int cmd,
    const std::string &loadBalanceElement,
    const std::string &request,
    const long tiemout,
    int &result,
    std::string &response)
{
    double start_time = Common::get_ms_time();
    std::string addr;
    if (!Send(cmd, loadBalanceElement, request, tiemout, addr, result, response))
    {
        bool is_do_hello = false;
        if (GrpcProtos::ResultType::ERR_Call_Service == result)
        {
            {
                std::unique_lock<std::shared_mutex> ul(m_AddrToHelloStatusMapRWMutex);
                if (!m_AddrToHelloStatusMap[addr].load())
                {
                    m_AddrToHelloStatusMap[addr].store(true);
                    is_do_hello = true;
                }
            }

            if (is_do_hello)
            {
                std::vector<std::string> addrList;
                addrList.push_back(addr);
                DoHello(addrList);

                std::unique_lock<std::shared_mutex> ul(m_AddrToHelloStatusMapRWMutex);
                m_AddrToHelloStatusMap[addr].store(false);
            }
        }

        LOG(WARNING) << "UnifiedClient::CallRemoteServer(): Call failed"
                     << ", addr = " << addr
                     << ", result = " << result
                     << ", response = " << response
                     << ", tiemout = " << Common::get_ms_time() - start_time << "ms";

        return false;
    }

    double deal_time = Common::get_ms_time() - start_time;
    PushRequestTime(deal_time);

    return true;
}

bool UnifiedClient::UpdateByAddr(const GrpcProtos::ServiceInfo &serviceInfo)
{
    const std::string &addr = serviceInfo.addr();

    {
        // 写锁, 出作用域自动释放
        std::unique_lock<std::shared_mutex> ul(m_AddrToServiceMapRWMutex);
        auto iter = m_AddrToServiceMap.find(addr);
        if (iter != m_AddrToServiceMap.end())
        {
            if (serviceInfo.group_tab() != iter->second.group_tab())
            {
                m_AddrToServiceMap.erase(iter);
                CloseClient(serviceInfo.addr(), serviceInfo.service_weight(), serviceInfo.connect_mode());
                return true;
            }
            else
            {
                iter->second.CopyFrom(serviceInfo);
            }
        }
        else
        {
            GrpcProtos::ServiceInfo &obj = m_AddrToServiceMap[addr];
            obj.CopyFrom(serviceInfo);
        }
    }

    {
        std::unique_lock<std::shared_mutex> ul(m_AddrToHelloStatusMapRWMutex);
        auto iter = m_AddrToHelloStatusMap.find(addr);
        if (iter == m_AddrToHelloStatusMap.end())
        {
            m_AddrToHelloStatusMap[addr].store(false);
        }
    }

    std::optional<semver::version> build_sv = semver::from_string_noexcept(m_RelySemver);

    std::optional<semver::version> req_sv = semver::from_string_noexcept(serviceInfo.semver());

    if (build_sv.has_value() &&
        req_sv.has_value() &&
        req_sv->major == build_sv->major &&
        req_sv >= build_sv &&
        serviceInfo.status() == GrpcProtos::ServiceStatus::Online)
    {
        OpenClient(serviceInfo.addr(), serviceInfo.service_weight(), serviceInfo.connect_mode());
    }
    else
    {
        CloseClient(serviceInfo.addr(), serviceInfo.service_weight(), serviceInfo.connect_mode());
    }

    return true;
}

void UnifiedClient::DoHello(const std::vector<std::string> &addrList)
{
    for (const auto &addr : addrList)
    {
        int result = GrpcProtos::ResultType::ERR_Unknown;
        std::string response;
        if (!Send(addr, GrpcProtos::CMD_HELLO, m_HelloRequest, 20L, result, response))
        {
            LOG(WARNING) << "UnifiedClient::DoHello(): Call failed"
                         << ", addr = " << addr
                         << ", result = " << result
                         << ", response = " << response;
            CloseByAddr(addr);
        }
        else
        {
            std::transform(response.begin(), response.end(), response.begin(), ::tolower);
            if (GrpcProtos::ResultType::OK != result || 0 != response.compare("ok"))
            {
                CloseByAddr(addr);
            }
            else
            {
                OpenByAddr(addr);
            }
        }
    }
}

bool UnifiedClient::OpenByAddr(const std::string &addr)
{
    std::unique_lock<std::shared_mutex> ul(m_AddrToServiceMapRWMutex);

    auto iter = m_AddrToServiceMap.find(addr);
    if (iter != m_AddrToServiceMap.end())
    {
        std::optional<semver::version> build_sv = semver::from_string_noexcept(m_RelySemver);

        std::optional<semver::version> req_sv = semver::from_string_noexcept(iter->second.semver());

        if (build_sv.has_value() &&
            req_sv.has_value() &&
            req_sv->major == build_sv->major &&
            req_sv >= build_sv)
        {
            iter->second.set_status(GrpcProtos::ServiceStatus::Online);
            OpenClient(iter->second.addr(), iter->second.service_weight(), iter->second.connect_mode());
        }
    }

    return true;
}

bool UnifiedClient::CloseByAddr(const std::string &addr)
{
    std::unique_lock<std::shared_mutex> ul(m_AddrToServiceMapRWMutex);

    auto iter = m_AddrToServiceMap.find(addr);
    if (iter != m_AddrToServiceMap.end())
    {
        iter->second.set_status(GrpcProtos::ServiceStatus::Offline);
        CloseClient(iter->second.addr(), iter->second.service_weight(), iter->second.connect_mode());
    }

    return true;
}

void UnifiedClient::GetWatchServiceInfo(GrpcProtos::WatchServiceInfo *watchServiceInfo)
{
    watchServiceInfo->set_service_type(m_ServiceType);

    // 读锁, 出作用域自动释放
    std::shared_lock<std::shared_mutex> sl(m_AddrToServiceMapRWMutex);
    for (auto iter = m_AddrToServiceMap.begin(); iter != m_AddrToServiceMap.end(); ++iter)
    {
        watchServiceInfo->add_service_list()->CopyFrom(iter->second);
    }
}
