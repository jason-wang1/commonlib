#include "TFModelBase.h"
#include "TFModelGrpc.h"
#include "TFTrans.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "HTTPClient/HTTPClient.h"

using namespace TDPredict;

TFModelBase::TFModelBase(
    const std::vector<std::string> &vec_tfs_addrs,
    const std::string &signature_name,
    const std::string &model_name,
    const int timeout,
    const bool use_dropout_keep,
    const int version)
{
    // 更新数据
    m_Timeout = timeout;
    m_ModelName = model_name;
    m_SignatureName = signature_name;
    m_TFSAddrList = vec_tfs_addrs;
    m_UseDropoutKeep = use_dropout_keep;
    m_Version = version;
}

// 批量预测函数
bool TFModelBase::predict_base(
    const std::vector<TFSInputData> &vec_tfs_inputs,
    std::vector<std::string> &vec_tfs_results) const noexcept
{
    // 判断输入为空, 就不需要调用tfs计算了.
    if (vec_tfs_inputs.empty())
    {
        LOG(INFO) << "predict() vec_tfs_inputs is empty.";
        return true;
    }

    // 获取模型基础数据
    const int vec_tfs_inputs_size = vec_tfs_inputs.size();
    if (int(vec_tfs_results.size()) != vec_tfs_inputs_size)
    {
        return false;
    }

    static const int per_batch_size = 256; // 每批数量
    int batch_count = (vec_tfs_inputs_size / per_batch_size) + ((vec_tfs_inputs_size % per_batch_size == 0) ? 0 : 1);
    std::vector<std::string> vec_post_url(batch_count);
    std::vector<std::string> vec_post_msg(batch_count); // post_msg的生命周期必须覆盖完整的predict请求过程, 否则会出bug, 你还查不出来...
    std::vector<std::shared_ptr<std::promise<int32_t>>> vec_client_code(batch_count);
    std::vector<std::shared_ptr<std::string>> vec_json_result(batch_count);
    std::vector<bool> vec_add_succ(batch_count, false);
    for (int batch_idx = 0; batch_idx != batch_count; batch_idx++)
    {
        const int begin = batch_idx * per_batch_size;
        const int end = std::min(vec_tfs_inputs_size, begin + per_batch_size);
        // 获取请求地址
        std::string &post_url = vec_post_url[batch_idx];
        {
            std::string tfs_addr = "";
            if (!GetRandAddr(m_TFSAddrList, tfs_addr))
            {
                continue;
            }
            post_url = make_tfs_request_URL(tfs_addr, m_ModelName);
        }
        std::string &post_msg = vec_post_msg[batch_idx];
        post_msg = make_tfs_batch_post_msg(m_SignatureName, vec_tfs_inputs, begin, end, m_UseDropoutKeep);

        // 发送请求
        vec_client_code[batch_idx] = std::make_shared<std::promise<int32_t>>();
        vec_json_result[batch_idx] = std::make_shared<std::string>();

        auto &client_code = vec_client_code[batch_idx];
        auto &json_result = vec_json_result[batch_idx];
        bool add_succ = HTTPClient::GetInstance()->add_post_request(post_url, {}, post_msg, m_Timeout, client_code, json_result);
        if (!add_succ)
        {
            LOG(ERROR) << "predict() HTTPClient add_post_request Failed"
                       << ", batch_idx = " << batch_idx
                       << ", post_url = " << post_url;
            continue;
        }

        vec_add_succ[batch_idx] = true;
    };

    // 计算等待超时时长
    auto wait_time_point = std::chrono::system_clock::now() + std::chrono::milliseconds(m_Timeout);

    for (int batch_idx = 0; batch_idx != batch_count; batch_idx++)
    {
        // 加入成功才等待返回值
        if (!vec_add_succ[batch_idx])
        {
            continue;
        }

        // 加入成功才等待返回值
        auto &client_code = vec_client_code[batch_idx];
        std::future<int32_t> future = client_code->get_future();
        if (future.wait_until(wait_time_point) == std::future_status::timeout)
        {
            // 等待超时, 进行一下项
            if (batch_idx == 0)
            {
                // 仅在第一次等待超时打印日志
                // 如果第一个请求没超时, 后面的请求大部分情况下也可认为没超时
                // 如果第一个请求超时了, 只需要打印一次日志
                LOG(ERROR) << "predict() future.wait_until() timeout"
                           << ", batch_idx = " << batch_idx
                           << ", post_url = " << vec_post_url[batch_idx]
                           << ", timeout = " << m_Timeout;
            }
            continue;
        }

        int32_t errCode = future.get();
        if (errCode != CURLE_OK)
        {
            LOG(ERROR) << "predict() post_request failed"
                       << ", post_url = " << vec_post_url[batch_idx]
                       << ", code = " << errCode;
            continue;
        }

        // 获取返回结果
        const int begin = batch_idx * per_batch_size;
        const int end = std::min(vec_tfs_inputs_size, begin + per_batch_size);
        const auto &json_result = *vec_json_result[batch_idx].get();
        if (!get_tfs_batch_post_result(json_result, begin, end, vec_tfs_results))
        {
            LOG(ERROR) << "predict() get_tfs_batch_post_result failed"
                       << ", batch_idx = " << batch_idx
                       << ", post_url = " << vec_post_url[batch_idx]
                       << ", model_name = " << m_ModelName
                       << ", json_result = " << get_format_json(json_result);
            continue;
        }
    }

    return true;
}

std::string TFModelBase::make_tfs_post_msg(const TFSInputData &tfs_input)
{
    return make_tfs_batch_post_msg(m_SignatureName, {tfs_input}, 0, 1, m_UseDropoutKeep);
}

bool TFModelBase::GetRandAddr(
    const std::vector<std::string> &vec_tfs_addrs,
    std::string &tfs_addr)
{
    // 获取请求地址
    if (vec_tfs_addrs.empty())
    {
        LOG(ERROR) << "GetRandAddr() addr_list is empty.";
        return false;
    }

    const int vec_tfs_addrs_size = vec_tfs_addrs.size();
    tfs_addr = (vec_tfs_addrs_size == 1)
                   ? vec_tfs_addrs[0]
                   : vec_tfs_addrs[(rand() % vec_tfs_addrs_size)];
    return !tfs_addr.empty();
}

std::string TFModelBase::make_tfs_batch_post_msg(
    const std::string &signatureName,
    const std::vector<TFSInputData> &vec_tfs_inputs,
    const int begin, const int end,
    const bool use_dropout_keep)
{
    // 构造tfs协议json数据
    rapidjson::StringBuffer json_buf;
    rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buf);
    json_writer.StartObject();
    json_writer.Key("signature_name");
    json_writer.String(signatureName.c_str());
    json_writer.Key("instances");
    json_writer.StartArray();
    for (int idx = begin; idx != end; idx++)
    {
        const auto &tfs_input = vec_tfs_inputs[idx];

        json_writer.StartObject();
        json_writer.Key("input_index");
        json_writer.StartArray();
        for (const auto &index : tfs_input.index)
        {
            json_writer.Int(index);
        }
        json_writer.EndArray();
        json_writer.Key("input_value");
        json_writer.StartArray();
        for (const auto &value : tfs_input.value)
        {
            json_writer.Double(value);
        }
        json_writer.EndArray();
        if (use_dropout_keep)
        {
            json_writer.Key("dropout_keep");
            json_writer.StartArray();
            json_writer.Int(1);
            json_writer.EndArray();
        }
        json_writer.EndObject();
    }
    json_writer.EndArray();
    json_writer.EndObject();
    return std::string(json_buf.GetString(), json_buf.GetSize());
}

bool TFModelBase::get_tfs_batch_post_result(
    const std::string &json_result,
    const int begin, const int end,
    std::vector<std::string> &vec_tfs_results)
{
    if (json_result.empty())
    {
        return false;
    }

    rapidjson::Document doc;
    if (doc.Parse(json_result.data()).HasParseError())
    {
        return false;
    }

    // 结果数组不存在
    if (!doc.HasMember("predictions") || !doc["predictions"].IsArray())
    {
        return false;
    }

    // 依次获取result
    auto predictions_array = doc["predictions"].GetArray();
    int predictions_array_size = predictions_array.Size();
    if ((end - begin) != predictions_array_size)
    {
        LOG(ERROR) << "get_tfs_batch_post_result() result size not equal"
                   << ", predictions_array_size = " << predictions_array_size
                   << ", begin = " << begin
                   << ", end = " << end;
        return false;
    }

    for (int idx = 0; idx != predictions_array_size; idx++)
    {
        rapidjson::Document test_doc;
        test_doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
        test_doc.AddMember("result_value", predictions_array[idx], allocator);

        rapidjson::StringBuffer json_buf;
        rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buf);
        test_doc.Accept(json_writer);
        vec_tfs_results[begin + idx] = std::string(json_buf.GetString(), json_buf.GetSize());
    }
    return true;
}

std::string TFModelBase::get_format_json(
    const std::string &json)
{
    rapidjson::Document doc;
    if (json.empty() || doc.Parse(json.data()).HasParseError())
    {
        return "";
    }
    rapidjson::StringBuffer json_buf;
    rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buf);
    doc.Accept(json_writer);
    return std::string(json_buf.GetString(), json_buf.GetSize());
}
