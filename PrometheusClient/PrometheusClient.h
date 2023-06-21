#pragma once
#include <memory>
#include <vector>
#include <map>
#include <atomic>
#include <thread>
#include <functional>
#include "Common/Singleton.h"

namespace prometheus
{
    template <typename T>
    class Family;

    class Registry;
    class Counter;
    class Histogram;
    class Summary;
    class Gauge;
    class Gateway;

    using CounterFamily = Family<Counter>;
    using HistogramFamily = Family<Histogram>;
    using SummaryFamily = Family<Summary>;
    using GaugeFamily = Family<Gauge>;
    using Labels = std::map<std::string, std::string>;
}

// Prometheus上报类
class PrometheusReport
{
public:
    // std::vector<std::string> vec_call_path;
    // [CMD], [CMD|Rank], [CMD|Rank|PreRank], [CMD|Rank|PreRank|ModelName]
    //
    // Path Example:
    // CMD
    // CMD|UserFeature
    // CMD|ItemFeature
    // CMD|Recall|RecallStrategy|ModelName
    // CMD|Rank|PreRank|ModelName
    // CMD|Display|DisplayStrategy
    //
    // Call Example:
    // PrometheusReport prom_report;
    // prom_report.start("CMD");
    // prom_report.start("UserFeature");
    // code = UserFeature.Run();
    // prom_report.end(code);
    // prom_report.start("ItemFeature");
    // code = MapFeature.Run();
    // prom_report.end(code);
    // prom_report.start("Rank");
    // code = MultiRank.Run() {
    //     prom_report.start("PreRank");
    //     code = PreRank.Run();
    //     prom_report.end(code);
    // };
    // prom_report.end(code);
    // prom_report.end(ret);

    void start(const std::string &path);
    void end(const int32_t code);

private:
    std::vector<std::string> vec_call_path;
    std::vector<double> vec_start_time;
};

using PrometheusReportPtr = std::shared_ptr<PrometheusReport>;
using ReportTask = std::function<void()>;

// 基于 prometheus-cpp 封装实现客户端
// 调用顺序:
// 1. AddTask
// 2. Init
// 3. ShutDown
class PrometheusClient : public Singleton<PrometheusClient>
{
public:
    PrometheusClient(token) { m_init = false; }
    virtual ~PrometheusClient() {}
    PrometheusClient(PrometheusClient &) = delete;
    PrometheusClient &operator=(const PrometheusClient &) = delete;

    bool AddTask(const ReportTask &task);

    bool Init(
        const bool report,
        const std::string &service_host,
        const std::string &service_name,
        const std::string &service_semver,
        const std::string &gateway_host,
        const std::string &gateway_port,
        const std::string &job_name,
        const int push_interval);

    void ShutDown();

public:
    // 请求次数统计
    void cmd_counter_inc(const std::string &path, const int32_t code);

    // 请求耗时 直方图/采样点分位图
    void cmd_durations_observer(const std::string &path, const double durations);

    // 用户态CPU使用率监控
    void user_cpu_usage_monitor(const double user_cpu_usage);

    // 内核态CPU使用率监控
    void kernel_cpu_usage_monitor(const double kernel_cpu_usage);

    // CPU使用率监控
    void cpu_usage_monitor(const double cpu_usage);

    // 虚拟内存使用率监控
    void virtual_memory_monitor(const double memory);

    // 物理内存使用率监控
    void resident_memory_monitor(const double memory);

    // 线程数量监控
    void threads_monitor(const double threads);

    // fd数量监控
    void fds_monitor(const double fds);

    // 联机实时地图/房间数量监控
    void online_map_count_monitor(const std::string &section, const int count);
    void online_room_count_monitor(const std::string &section, const int count);

    // 联机实时倒排索引监控
    void online_invert_index_count_monitor(const std::string &name, const int count);
    void clean_online_invert_index_monitor();

protected:
    bool ProcPidStat();   // 进程状态获取函数
    bool ProcPidFd();     // 进程Fd获取函数
    void TimerPushFunc(); // 定时推送函数

private:
    pid_t m_Pid;

    long m_PageSize;
    double m_MemTotal_B;

    long m_ClockTicks;
    long m_ProcessorsNum;

private:
    std::atomic<bool> m_init;
    bool m_report = false;
    prometheus::Labels m_service_labels;
    std::shared_ptr<prometheus::Registry> m_spRegistry = nullptr;

    // 采集CMD信息
    prometheus::CounterFamily *m_pCounterFamily;     // 计数器
    prometheus::HistogramFamily *m_pHistogramFamily; // 直方图
    prometheus::SummaryFamily *m_pSummaryFamily;     // 采样点分位图

    // 采集机器信息
    prometheus::GaugeFamily *m_pMachineCPUFamily;    // 采集机器CPU数量
    prometheus::GaugeFamily *m_pMachineMemoryFamily; // 采集机器内存大小

    // 采集服务信息
    prometheus::GaugeFamily *m_pUserCPUFamily;   // 采集用户态CPU波动指标
    prometheus::GaugeFamily *m_pKernelCPUFamily; // 采集内核态CPU波动指标
    prometheus::GaugeFamily *m_pCPUFamily;       // 采集CPU波动指标
    prometheus::GaugeFamily *m_pVMMemFamily;     // 采集虚拟内存波动指标
    prometheus::GaugeFamily *m_pRSSMemFamily;    // 采集物理内存波动指标
    prometheus::GaugeFamily *m_pThreadFamily;    // 采集线程数量波动指标
    prometheus::GaugeFamily *m_pFdFamily;        // 采集Fd数量波动指标

    // 采集服务上报信息
    prometheus::GaugeFamily *m_pMapCountFamily;         // 线上实时地图数量
    prometheus::GaugeFamily *m_pRoomCountFamily;        // 线上实时房间数量
    prometheus::GaugeFamily *m_pInvertIndexCountFamily; // 线上实时倒排数量

    // 上报任务
    std::vector<ReportTask> m_vecReportTask;

    // Push
    std::shared_ptr<prometheus::Gateway> m_spPushGateway = nullptr;
    std::mutex m_pushMutex;
    std::thread m_pushThread;
    int m_pushInterval = 10;
};
