#pragma once
#include <string>
#include <future> // std::async, std::future, std::promise
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include <queue>
#include <unordered_map>
#include "Common/Singleton.h"
#include "curl/curl.h"

struct HttpTaskData
{
    CURL *handle = nullptr;
    std::shared_ptr<std::promise<int32_t>> spCode = nullptr;
    std::shared_ptr<std::string> spResult = nullptr;
    long ms_timestamp = 0; // 超时时间判断
};

using SPHttpTaskData = std::shared_ptr<HttpTaskData>;
using RunningTaskMap = std::unordered_map<CURL *, SPHttpTaskData>;

class HTTPClient : public Singleton<HTTPClient>
{
public:
    HTTPClient(token);
    virtual ~HTTPClient();
    HTTPClient(const HTTPClient &) = delete;
    HTTPClient &operator=(const HTTPClient &) = delete;

    bool Init();
    void ShutDown();

public:
    static bool post_request(
        const std::string &url,
        const std::vector<std::string> vecHeader,
        const std::string &post_msg,
        const int timeout_ms,
        std::string &result);

    bool add_post_request(
        const std::string &url,
        const std::vector<std::string> vecHeader,
        const std::string &post_msg,
        const int timeout_ms,
        std::shared_ptr<std::promise<int32_t>> &spCode,
        std::shared_ptr<std::string> &spResult);

protected:
    // 生产者 - grpc线程
    bool add_wait_task(
        CURL *handle,
        const int timeout_ms,
        std::shared_ptr<std::promise<int32_t>> &spCode,
        std::shared_ptr<std::string> &spResult);

    // 消费者 - 工作线程
    void thread_work_func(const int thread_idx);

    int add_request_task(
        const int thread_idx,
        CURLM *multi_handle,
        RunningTaskMap &map_running_task);

public:
    struct WorkThread
    {
        // 任务列表
        std::mutex wait_task_mutex;
        std::queue<SPHttpTaskData> wait_task_queue;

        // 线程管理
        std::thread thread;
        std::mutex mtx;
        std::condition_variable cv;

        void get_wait_task(std::vector<SPHttpTaskData> &vec_wait_task);
    };

private:
    // 工作线程
    static constexpr int ThreadCount = 5;
    WorkThread g_workThread[ThreadCount];

    // 初始化标志
    std::atomic<bool> g_init = false;

    // http 请求参数
    struct curl_slist *header_list = nullptr;
};
