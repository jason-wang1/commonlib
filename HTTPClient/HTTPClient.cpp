#include "HTTPClient.h"
#include "curl/curl.h"
#include "glog/logging.h"
#include "Common/Function.h"

static size_t curl_write_func(void *contents, size_t size, size_t nmemb, std::string *s) noexcept
{
    size_t new_length = size * nmemb;
    try
    {
        s->append((char *)contents, new_length);
    }
    catch (std::bad_alloc &e)
    {
        return 0; // handle memory problem
    }
    return new_length;
}

HTTPClient::HTTPClient(token)
{
    std::string header = "Content-Type: application/json;charset=UTF-8";
    header_list = curl_slist_append(header_list, header.c_str());
}

HTTPClient::~HTTPClient()
{
    curl_slist_free_all(header_list);
}

bool HTTPClient::Init()
{
    g_init = true;
    for (int idx = 0; idx != ThreadCount; idx++)
    {
        auto thread_func = std::bind(&HTTPClient::thread_work_func, this, idx);
        g_workThread[idx].thread = std::thread(thread_func);
    }
    return true;
}

void HTTPClient::ShutDown()
{
    if (g_init)
    {
        g_init = false;

        // 唤醒所有工作线程
        for (int idx = 0; idx != ThreadCount; idx++)
        {
            std::unique_lock<std::mutex> lck(g_workThread[idx].mtx);
            g_workThread[idx].cv.notify_all();
        }

        // 等待工作线程退出
        for (int idx = 0; idx != ThreadCount; idx++)
        {
            if (g_workThread[idx].thread.joinable())
            {
                g_workThread[idx].thread.join();
            }
        }
    }
}

bool HTTPClient::post_request(
    const std::string &url,
    const std::vector<std::string> vecHeader,
    const std::string &post_msg,
    const int timeout_ms,
    std::string &result)
{
    CURL *handle = curl_easy_init();
    if (handle == NULL)
    {
        LOG(ERROR) << "post_request() curl_easy_init falid.";
        return false;
    }

    // 请求头
    struct curl_slist *header_list = NULL;
    if (vecHeader.empty())
    {
        header_list = curl_slist_append(header_list, "Content-Type:application/json;charset=UTF-8");
    }
    else
    {
        for (const auto &header : vecHeader)
        {
            header_list = curl_slist_append(header_list, header.c_str());
        }
    }
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, header_list);

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());                  // 请求地址
    curl_easy_setopt(handle, CURLOPT_PRIVATE, url.c_str());              // 私密数据指针
    curl_easy_setopt(handle, CURLOPT_POST, 1);                           // 本次操作为POST
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, post_msg.c_str());      // 设置要POST的JSON数据
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, post_msg.size());    // 设置要POST的JSON数据长度
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_func);    // 处理返回数据函数
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &result);                // 接收返回数据参数
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, timeout_ms);            // 接收数据时超时设置
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);                 // 返回头部有Location, 则继续请求Location对应的数据
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 1);                      // 查找次数, 防止查找太深
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);                      // 禁用任何信号/警报处理程序
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2); // 使用 HTTP/2
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);

    // TCP Keep Alive
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);  // 为这个传输启用 TCP keep-alive
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPIDLE, 120L); // keep-alive 空闲时间为 120 秒
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPINTVL, 60L); // keep-alive 探测之间的间隔时间：60 秒

    // 限制连接生命周期, 禁止一个连接生存过长时间
    curl_easy_setopt(handle, CURLOPT_MAXLIFETIME_CONN, 30L); // 重用缓存连接的最长时间, Curl-Ver 7.80.0
    curl_easy_setopt(handle, CURLOPT_MAXAGE_CONN, 30L);      // 连接处于空闲状态的最长时间, Curl-Ver 7.65.0

    CURLcode errCode = curl_easy_perform(handle);

    curl_slist_free_all(header_list);
    curl_easy_cleanup(handle);

    // 针对超时的情况单独Log
    if (errCode == CURLE_OPERATION_TIMEDOUT)
    {
        LOG(ERROR) << "post_request() request timeout"
                   << ", url = " << url
                   << ", code = " << errCode
                   << "(CURLE_OPERATION_TIMEDOUT)";
        return false;
    }

    if (errCode != CURLE_OK)
    {
        LOG(ERROR) << "post_request() request failed"
                   << ", url = " << url
                   << ", code = " << errCode;
        return false;
    }
    return true;
}

bool HTTPClient::add_post_request(
    const std::string &url,
    const std::vector<std::string> vecHeader,
    const std::string &post_msg,
    const int timeout_ms,
    std::shared_ptr<std::promise<int32_t>> &spCode,
    std::shared_ptr<std::string> &spResult)
{
    if (spCode == nullptr)
    {
        spResult->assign("spCode is nullptr");
        return false;
    }

    if (!g_init)
    {
        spCode->set_value(CURLE_FAILED_INIT);
        spResult->assign("HTTPClient init failed");
        return false;
    }

    CURL *handle = curl_easy_init();
    if (handle == nullptr)
    {
        spCode->set_value(CURLE_FAILED_INIT);
        spResult->assign("curl_easy_init failed");
        return false;
    }

    // 请求头
    struct curl_slist *header_list = NULL;
    if (vecHeader.empty())
    {
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, this->header_list); // 设置Header
    }
    else
    {
        for (const auto &header : vecHeader)
        {
            header_list = curl_slist_append(header_list, header.c_str());
        }
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, header_list);
    }

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());                  // 请求地址
    curl_easy_setopt(handle, CURLOPT_PRIVATE, url.c_str());              // 私密数据指针
    curl_easy_setopt(handle, CURLOPT_POST, 1);                           // 本次操作为POST
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, post_msg.c_str());      // 设置要POST的JSON数据
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, post_msg.size());    // 设置要POST的JSON数据长度
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, this->header_list);     // 设置Header
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_func);    // 处理返回数据函数
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, spResult.get());         // 接收返回数据参数
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, timeout_ms);            // 接收数据时超时设置
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);                 // 返回头部有Location, 则继续请求Location对应的数据
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 1);                      // 查找次数，防止查找太深
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);                      // 禁用任何信号/警报处理程序
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2); // 使用 HTTP/2
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);

    // curl_easy_setopt(handle, CURLOPT_PIPEWAIT, 1L); // 等待连接从而进行流水线 Or 多路复用

    // TCP Keep Alive
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);  // 为这个传输启用 TCP keep-alive
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPIDLE, 120L); // keep-alive 空闲时间为 120 秒
    curl_easy_setopt(handle, CURLOPT_TCP_KEEPINTVL, 60L); // keep-alive 探测之间的间隔时间：60 秒

    // 限制连接生命周期, 禁止一个连接生存过长时间
    curl_easy_setopt(handle, CURLOPT_MAXLIFETIME_CONN, 30L); // 重用缓存连接的最长时间, Curl-Ver 7.80.0
    curl_easy_setopt(handle, CURLOPT_MAXAGE_CONN, 30L);      // 连接处于空闲状态的最长时间, Curl-Ver 7.65.0

    if (!add_wait_task(handle, timeout_ms, spCode, spResult))
    {
        spCode->set_value(CURLE_FAILED_INIT);
        spResult->assign("add_wait_task failed");
        return false;
    }
    return true;
}

bool HTTPClient::add_wait_task(
    CURL *handle,
    const int timeout_ms,
    std::shared_ptr<std::promise<int32_t>> &spCode,
    std::shared_ptr<std::string> &spResult)
{
    auto spTaskData = std::make_shared<HttpTaskData>();
    if (spTaskData == nullptr)
    {
        return false;
    }
    spTaskData->spCode = spCode;
    spTaskData->spResult = spResult;
    spTaskData->handle = handle;
    spTaskData->ms_timestamp = Common::get_ms_timestamp() + timeout_ms;

    // 随机分配一个线程
    const int idx = rand() % ThreadCount;

    int cur_wait_count = 0;
    {
        std::lock_guard<std::mutex> lg(g_workThread[idx].wait_task_mutex);
        g_workThread[idx].wait_task_queue.push(std::move(spTaskData));
        cur_wait_count = g_workThread[idx].wait_task_queue.size();
    }

    if (cur_wait_count > 30)
    {
        LOG(WARNING) << "HTTPClient add_wait_task, cur_wait_count = " << cur_wait_count;
    }
    else if (cur_wait_count > 100)
    {
        LOG(ERROR) << "HTTPClient add_wait_task, cur_wait_count = " << cur_wait_count;
    }

    // 有新任务进入, 唤醒对应工作线程
    {
        std::unique_lock<std::mutex> lck(g_workThread[idx].mtx);
        g_workThread[idx].cv.notify_one();
    }
    return true;
}

void HTTPClient::WorkThread::get_wait_task(std::vector<SPHttpTaskData> &vec_wait_task)
{
    std::lock_guard<std::mutex> lg(wait_task_mutex);
    vec_wait_task.reserve(wait_task_queue.size());
    while (!wait_task_queue.empty())
    {
        vec_wait_task.push_back(wait_task_queue.front());
        wait_task_queue.pop();
    }
}

int HTTPClient::add_request_task(
    const int thread_idx,
    CURLM *multi_handle,
    RunningTaskMap &map_running_task)
{
    // 获取等待任务
    std::vector<SPHttpTaskData> vec_wait_task;
    g_workThread[thread_idx].get_wait_task(vec_wait_task);

    // 将没有超时的请求任务加入处理
    int add_count = 0;
    const long cur_timestamp = Common::get_ms_timestamp();
    for (int idx = 0, size = vec_wait_task.size(); idx != size; idx++)
    {
        // 判断请求超时
        auto &spTask = vec_wait_task[idx];
        if (cur_timestamp > spTask->ms_timestamp)
        {
            spTask->spCode->set_value(CURLE_OPERATION_TIMEDOUT);
            curl_easy_cleanup(spTask->handle);
            continue;
        }

        // 加入处理
        curl_multi_add_handle(multi_handle, spTask->handle);
        map_running_task[spTask->handle] = spTask;
        add_count += 1;
    }
    return add_count;
}

void HTTPClient::thread_work_func(const int thread_idx)
{
    CURLM *multi_handle = nullptr;
    RunningTaskMap map_running_task;
    int still_alive = 0; // 正在 multi_handle 中执行的 easy_handle 数量
    CURLMsg *msg;
    int msgs_left = -1;

    multi_handle = curl_multi_init();

    // http2 多路复用 / http1.1 pipeline
    // 从7.62.0开始, 废弃 CURLPIPE_HTTP1, 默认启用 CURLPIPE_MULTIPLEX
    curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

    // 连接缓存池的大小
    // 在连接完成使用后可以保留在<连接缓存池>中的同时打开的最大连接数.
    // 当缓存已满时, curl会关闭缓存中最老的一个, 防止打开的连接数增加。
    // P.s> 经过实验, 这个参数可以解决短连接 TIME_WAIT 的问题
    //    但是会导致因为缓存连接, load_balance上下线机器不生效问题
    //    这里的解决方案之一是, 去掉load_balance, 用直连
    curl_multi_setopt(multi_handle, CURLMOPT_MAXCONNECTS, 300);

    // 到一台主机的最大(管道)连接数, 达到限制时, 会话将处于挂起状态, 直到连接可用.
    // 默认最大值为0(无限制),
    // 但为了向后兼容, 当 CURLMOPT_PIPELINING 为1时将其设置为0不会被视为无限制,
    // 它将仅打开1个连接并尝试对其进行管道传输.
    // curl_multi_setopt(multi_handle, CURLMOPT_MAX_HOST_CONNECTIONS, 1000);

    // 最大同时打开的连接数
    // 对于每个新会话, libcurl将打开一个新连接, 直至达到限制。
    // 当达到限制时, 会话将处于挂起状态, 直到有可用连接。
    // 默认值为0, 表示没有限制
    // curl_multi_setopt(multi_handle, CURLMOPT_MAX_TOTAL_CONNECTIONS, 1000);

    while (g_init || still_alive)
    {
        // 添加任务
        const int add_task_count = add_request_task(thread_idx, multi_handle, map_running_task);

        // 没有新任务 && 没有正在执行的任务
        if (g_init && !add_task_count && !still_alive)
        {
            // 等待新任务唤醒线程
            std::unique_lock<std::mutex> lck(g_workThread[thread_idx].mtx);
            g_workThread[thread_idx].cv.wait_for(lck, std::chrono::milliseconds(5));
            continue;
        }

        // 对已经添加到 multi_handle 的 easy_handle 执行传输, 该函数在执行完成后立即返回
        curl_multi_perform(multi_handle, &still_alive);

        // 询问是否有传输完成的消息
        // 从函数获取的消息会被内部队列删除, 因此再次调用函数不会返回相同的消息
        // 该函数将在每次调用时返回新消息, 直到内部队列被清空
        while ((msg = curl_multi_info_read(multi_handle, &msgs_left)))
        {
            // 单次传输任务完成后:
            // 1. 使用 curl_multi_remove_handle 删除 easy_handle
            // 2. 将其关闭或设置新参数再次使用:
            //   2-1. 使用 curl_easy_cleanup 将其关闭
            //   2-2. 设置新选项并使用 curl_multi_add_handle 再次添加, 开始新一次传输
            // P.s> 这里说明可以在单次任务完成后添加新句柄
            if (msg->msg == CURLMSG_DONE)
            {
                auto it = map_running_task.find(msg->easy_handle);
                if (it != map_running_task.end())
                {
                    it->second->spCode->set_value(msg->data.result);
                    map_running_task.erase(it);
                }
                curl_multi_remove_handle(multi_handle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
            }
            add_request_task(thread_idx, multi_handle, map_running_task);
        }

        if (still_alive)
        {
            // 轮询 multi_handle 中 easy_handle 使用的所有文件描述符(socket)
            // 它将阻塞, 直到在至少一个句柄上检测到活动 或timeout_ms超时
            curl_multi_wait(multi_handle, NULL, 0, 5, NULL);
        }
    }
    curl_multi_cleanup(multi_handle);
}