#include "PrometheusClient.h"
#include "Common/Function.h"
#include "glog/logging.h"
#include "assert.h"

// prometheus-cpp/core/include
#include "prometheus/counter.h"
#include "prometheus/histogram.h"
#include "prometheus/summary.h"
#include "prometheus/registry.h"

// prometheus-cpp/push/include
#include "prometheus/gateway.h"

using namespace prometheus;

void PrometheusReport::start(const std::string &path)
{
    if (vec_call_path.empty())
    {
        vec_call_path.emplace_back(path);
    }
    else
    {
        vec_call_path.emplace_back(vec_call_path.back() + "|" + path);
    }
    vec_start_time.emplace_back(Common::get_ms_time());
}

void PrometheusReport::end(const int32_t code)
{
    assert(!vec_call_path.empty());
    assert(!vec_start_time.empty());
    assert(vec_call_path.size() == vec_start_time.size());

    const std::string &path = vec_call_path.back();
    double start_time = vec_start_time.back();
    double end_time = Common::get_ms_time();

    // 上报Prometheus
    auto prom_client = PrometheusClient::GetInstance();
    prom_client->cmd_counter_inc(path, code);
    prom_client->cmd_durations_observer(path, end_time - start_time);

    vec_call_path.pop_back();
    vec_start_time.pop_back();
}

bool PrometheusClient::Init(
    const bool report,
    const std::string &service_host,
    const std::string &service_name,
    const std::string &service_semver,
    const std::string &gateway_host,
    const std::string &gateway_port,
    const std::string &job_name,
    const int push_interval)
{
    if (m_init == true)
    {
        // 重复初始化
        return false;
    }

    this->m_report = report;
    if (!m_report)
    {
        m_init = true;
        return true;
    }

    this->AddTask([&]()
                  { ProcPidStat(); });
    this->AddTask([&]()
                  { ProcPidFd(); });

    this->m_service_labels = {
        {"host", service_host},
        {"service_name", service_name},
        {"semver", service_semver},
    };

    // 获取进程ID
    m_Pid = getpid();

    // 获取内存页大小
    m_PageSize = sysconf(_SC_PAGESIZE);
    m_MemTotal_B = m_PageSize * sysconf(_SC_PHYS_PAGES);

    // 获取时钟滴答数量
    m_ClockTicks = sysconf(_SC_CLK_TCK);

    // 获取处理器数量
    m_ProcessorsNum = sysconf(_SC_NPROCESSORS_ONLN);

    // 默认行为 insert_behavior = Merge
    m_spRegistry = std::make_shared<Registry>(prometheus::Registry::InsertBehavior::Merge);

    m_pCounterFamily = &prometheus::BuildCounter()
                            .Name("cmd_requests_total")
                            .Labels(m_service_labels)
                            .Register(*m_spRegistry);

    m_pHistogramFamily = &prometheus::BuildHistogram()
                              .Name("cmd_durations_histogram_seconds")
                              .Labels(m_service_labels)
                              .Register(*m_spRegistry);

    m_pSummaryFamily = &prometheus::BuildSummary()
                            .Name("cmd_durations_summary_seconds")
                            .Labels(m_service_labels)
                            .Register(*m_spRegistry);

    m_pMachineCPUFamily = &prometheus::BuildGauge()
                               .Name("machine_cpu_cores")
                               .Labels(m_service_labels)
                               .Register(*m_spRegistry);
    m_pMachineCPUFamily->Add({}).Set(m_ProcessorsNum);

    m_pMachineMemoryFamily = &prometheus::BuildGauge()
                                  .Name("machine_memory_bytes")
                                  .Labels(m_service_labels)
                                  .Register(*m_spRegistry);
    m_pMachineMemoryFamily->Add({}).Increment(m_MemTotal_B);

    m_pUserCPUFamily = &prometheus::BuildGauge()
                            .Name("service_user_cpu_seconds_total")
                            .Labels(m_service_labels)
                            .Register(*m_spRegistry);

    m_pKernelCPUFamily = &prometheus::BuildGauge()
                              .Name("service_kernel_cpu_seconds_total")
                              .Labels(m_service_labels)
                              .Register(*m_spRegistry);

    m_pCPUFamily = &prometheus::BuildGauge()
                        .Name("service_cpu_seconds_total")
                        .Labels(m_service_labels)
                        .Register(*m_spRegistry);

    m_pRSSMemFamily = &prometheus::BuildGauge()
                           .Name("service_resident_memory_bytes")
                           .Labels(m_service_labels)
                           .Register(*m_spRegistry);

    m_pVMMemFamily = &prometheus::BuildGauge()
                          .Name("service_virtual_memory_bytes")
                          .Labels(m_service_labels)
                          .Register(*m_spRegistry);

    m_pThreadFamily = &prometheus::BuildGauge()
                           .Name("service_threads")
                           .Labels(m_service_labels)
                           .Register(*m_spRegistry);

    m_pFdFamily = &prometheus::BuildGauge()
                       .Name("service_fds")
                       .Labels(m_service_labels)
                       .Register(*m_spRegistry);

    m_pMapCountFamily = &prometheus::BuildGauge()
                             .Name("online_map_count")
                             .Labels(m_service_labels)
                             .Register(*m_spRegistry);

    m_pRoomCountFamily = &prometheus::BuildGauge()
                              .Name("online_room_count")
                              .Labels(m_service_labels)
                              .Register(*m_spRegistry);

    m_pInvertIndexCountFamily = &prometheus::BuildGauge()
                                     .Name("online_invert_index_count")
                                     .Labels(m_service_labels)
                                     .Register(*m_spRegistry);

    // Init PushGateway
    const std::string hostname = Common::get_hostname();
    m_spPushGateway = std::make_shared<Gateway>(
        gateway_host, gateway_port, job_name,
        Gateway::GetInstanceLabel(hostname));
    m_spPushGateway->RegisterCollectable(m_spRegistry);
    m_pushInterval = push_interval;

    // 初始化成功, 启动线程
    m_init = true;
    m_pushThread = std::thread(std::bind(&PrometheusClient::TimerPushFunc, this));

    return true;
}

bool PrometheusClient::AddTask(const ReportTask &task)
{
    if (!m_init)
    {
        m_vecReportTask.emplace_back(task);
        return true;
    }
    return false;
}

void PrometheusClient::ShutDown()
{
    if (m_init)
    {
        {
            // 清理数据
            std::lock_guard<std::mutex> lg(m_pushMutex);
            if (m_spPushGateway != nullptr)
            {
                m_spPushGateway->DeleteForInstance();
            }
            m_report = false;
        }

        m_init = false;
        if (m_pushThread.joinable())
        {
            m_pushThread.join();
        }

        m_pCounterFamily = nullptr;
        m_pHistogramFamily = nullptr;
        m_pSummaryFamily = nullptr;

        m_pMachineCPUFamily = nullptr;
        m_pMachineMemoryFamily = nullptr;

        m_pUserCPUFamily = nullptr;
        m_pKernelCPUFamily = nullptr;
        m_pCPUFamily = nullptr;
        m_pVMMemFamily = nullptr;
        m_pRSSMemFamily = nullptr;
        m_pThreadFamily = nullptr;
        m_pFdFamily = nullptr;

        m_pMapCountFamily = nullptr;
        m_pRoomCountFamily = nullptr;
        m_pInvertIndexCountFamily = nullptr;

        m_spPushGateway = nullptr;
        m_spRegistry = nullptr;

        m_vecReportTask.clear();
    }
}

void PrometheusClient::cmd_counter_inc(
    const std::string &path,
    const int32_t code)
{
    if (m_report && m_pCounterFamily != nullptr)
    {
        m_pCounterFamily
            ->Add({{"path", path}, {"code", std::to_string(code)}})
            .Increment();
    }
}

void PrometheusClient::cmd_durations_observer(
    const std::string &path,
    const double durations)
{
    if (!m_report)
    {
        return;
    }

    static const auto bucket_boundaries =
        Histogram::BucketBoundaries{3, 5, 10, 30, 50, 100, 200, 500, 1000};

    static const auto quantiles =
        Summary::Quantiles{{0.5, 0.05}, {0.9, 0.01}, {0.99, 0.001}, {0.999, 0.0001}, {0.9999, 0.00001}};

    if (m_pHistogramFamily != nullptr)
    {
        m_pHistogramFamily->Add({{"path", path}}, bucket_boundaries).Observe(durations);
    }

    if (m_pSummaryFamily != nullptr)
    {
        m_pSummaryFamily->Add({{"path", path}}, quantiles).Observe(durations);
    }
}

void PrometheusClient::user_cpu_usage_monitor(const double user_cpu_usage)
{
    if (m_report && m_pUserCPUFamily != nullptr)
    {
        m_pUserCPUFamily->Add({}).Set(user_cpu_usage);
    }
}

void PrometheusClient::kernel_cpu_usage_monitor(const double kernel_cpu_usage)
{
    if (m_report && m_pKernelCPUFamily != nullptr)
    {
        m_pKernelCPUFamily->Add({}).Set(kernel_cpu_usage);
    }
}

void PrometheusClient::cpu_usage_monitor(const double cpu_usage)
{
    if (m_report && m_pCPUFamily != nullptr)
    {
        m_pCPUFamily->Add({}).Set(cpu_usage);
    }
}

void PrometheusClient::virtual_memory_monitor(const double memory)
{
    if (m_report && m_pVMMemFamily != nullptr)
    {
        m_pVMMemFamily->Add({}).Set(memory);
    }
}

void PrometheusClient::resident_memory_monitor(const double memory)
{
    if (m_report && m_pRSSMemFamily != nullptr)
    {
        m_pRSSMemFamily->Add({}).Set(memory);
    }
}

void PrometheusClient::threads_monitor(const double threads)
{
    if (m_report && m_pThreadFamily != nullptr)
    {
        m_pThreadFamily->Add({}).Set(threads);
    }
}

void PrometheusClient::fds_monitor(const double fds)
{
    if (m_report && m_pFdFamily != nullptr)
    {
        m_pFdFamily->Add({}).Set(fds);
    }
}

void PrometheusClient::online_map_count_monitor(
    const std::string &section,
    const int count)
{
    if (m_report && m_pMapCountFamily != nullptr)
    {
        m_pMapCountFamily->Add({{"section", section}}).Set(count);
    }
}

// 联机实时房间数量监控
void PrometheusClient::online_room_count_monitor(
    const std::string &section,
    const int count)
{
    if (m_report && m_pRoomCountFamily != nullptr)
    {
        m_pRoomCountFamily->Add({{"section", section}}).Set(count);
    }
}

// 联机实时倒排索引数量监控
void PrometheusClient::online_invert_index_count_monitor(
    const std::string &name,
    const int count)
{
    if (m_report && m_pInvertIndexCountFamily != nullptr)
    {
        m_pInvertIndexCountFamily->Add({{"name", name}}).Set(count);
    }
}

void PrometheusClient::clean_online_invert_index_monitor()
{
    std::vector<prometheus::Labels> vecLabels;
    if (m_report && m_pInvertIndexCountFamily != nullptr)
    {
        // 获取所有上报的名称
        const auto collected = m_pInvertIndexCountFamily->Collect();
        for (const auto &collect : collected)
        {
            for (const auto &metric : collect.metric)
            {
                for (const auto &label : metric.label)
                {
                    if (label.name == "name")
                    {
                        vecLabels.push_back({{"name", label.value}});
                    }
                }
            }
        }

        // 清理掉 vecLabels
        for (const auto &label : vecLabels)
        {
            auto &guage = m_pInvertIndexCountFamily->Add(label);
            m_pInvertIndexCountFamily->Remove(&guage);
        }
    }
}

bool PrometheusClient::ProcPidStat()
{
    // 读取并解析 /proc/[pid]/stat 文件
    std::string fileName = "/proc/";
    fileName.append(std::to_string(m_Pid));
    fileName.append("/stat");

    std::string buffer;
    if (!Common::SystemCall_ReadFile(fileName, buffer))
    {
        LOG(WARNING) << "PrometheusClient::ProcPidStat() Read File Failed"
                     << ", fileName = " << fileName;
        return false;
    }

    std::vector<std::string> task_stat;
    Common::SplitString(buffer, ' ', task_stat);

    size_t key_part_size = task_stat.size();
    if (key_part_size < 52)
    {
        LOG(WARNING) << "PrometheusClient::ProcPidStat() Task Stat Split Error"
                     << ", buffer = [ " << buffer << "]";
        return false;
    }

    // 获取cpu使用率
    {
        long user_cpu_usage_time = atol(task_stat[13].c_str());
        const double user_cpu_usage = double(user_cpu_usage_time) / m_ClockTicks;

        // 上报用户态CPU使用率
        user_cpu_usage_monitor(user_cpu_usage);

        long kernel_cpu_usage_time = atol(task_stat[14].c_str());
        const double kernel_cpu_usage = double(kernel_cpu_usage_time) / m_ClockTicks;

        // 上报内核态CPU使用率
        kernel_cpu_usage_monitor(kernel_cpu_usage);

        const double cpu_usage = user_cpu_usage + kernel_cpu_usage;

        // 上报CPU使用率
        cpu_usage_monitor(cpu_usage);
    }

    // 获取使用虚拟内存数量
    {
        const long vm_size_B = atol(task_stat[22].c_str());

        // 上报虚拟内存
        virtual_memory_monitor(vm_size_B);
    }

    // 获取使用物理内存数量
    {
        const long rss_page = atol(task_stat[23].c_str());
        const long rss_size_B = rss_page * m_PageSize;

        // 上报物理内存
        resident_memory_monitor(rss_size_B);
    }

    // 获取线程数量
    {
        const long num_threads = atol(task_stat[19].c_str());

        // 上报线程数量
        threads_monitor(num_threads);
    }

    return true;
}

bool PrometheusClient::ProcPidFd()
{
    // 获取 /proc/[pid]/fd 目录文件数量
    std::string dirPath = "/proc/";
    dirPath.append(std::to_string(m_Pid));
    dirPath.append("/fd");

    int fds;
    if (!Common::SystemCall_GetFileNumInDir(dirPath, fds))
    {
        LOG(WARNING) << "PrometheusClient::ProcPidFd() Get FileNum Failed"
                     << ", dirPath = " << dirPath;
        return false;
    }

    // 上报fd数量
    fds_monitor(fds);

    return true;
}

void PrometheusClient::TimerPushFunc()
{
    long loop = 0; // 从0开始, 启动就可以执行一次上报
    while (m_report && m_init)
    {
        // 每N秒执行一次 Push, 直到ShutDown
        if ((loop % m_pushInterval) == 0)
        {
            std::lock_guard<std::mutex> lg(m_pushMutex);
            if (m_report)
            {
                // 按顺序调用任务
                for (auto &task : m_vecReportTask)
                {
                    task();
                }

                int push_code = m_spPushGateway->Push();
                if (push_code != 200)
                {
                    LOG(ERROR) << "TimerPushFunc() PushGateway push_code = " << push_code;
                }
            }
        }
        loop = (loop + 1) % 3600;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}