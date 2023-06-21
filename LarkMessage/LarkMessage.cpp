#include "LarkMessage.h"

void LarkMessage::SetOther(const std::string &other)
{
    m_other = other;
}

void LarkMessage::SetTitle(const std::string &title)
{
    m_title = title;
}

void LarkMessage::AddText(const std::string &key, const std::string &text)
{
    m_text_list.push_back(std::make_pair(key, text));
}

void LarkMessage::AssemblyData(std::string &outData)
{
    //构造json数据
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> logWriter(buffer);

    logWriter.StartObject();
    logWriter.Key("msgtype");
    logWriter.String("markdown");
    logWriter.Key("markdown");
    logWriter.StartObject();
    logWriter.Key("title");
    logWriter.String(m_title.c_str(), m_title.size());

    std::string text_info = "### " + m_title;
    for (const auto &ps: m_text_list)
    {
        text_info.append("\n\n " + ps.first + ps.second);
    }
    if(!m_other.empty())
    {
        text_info.append("\n\n " + m_other);
    }
    logWriter.Key("text");
    logWriter.String(text_info.c_str(), text_info.size());

    logWriter.EndObject();
    logWriter.EndObject();

    // 输出数据
    outData.clear();
    outData.append(buffer.GetString(), buffer.GetSize());
}

static size_t CurlCallbackWirteFunc(char *buffer, size_t size, size_t nmemb, void *userp)
{
    std::string &content = *(static_cast<std::string *>(userp));
    size_t len = size * nmemb;
    for (size_t i = 0; i < len; ++i)
    {
        content += *buffer;
        ++buffer;
    }
    return len;
}

bool LarkMessage::Send(const std::string &webhook, std::string &retMsg)
{
    std::string jsonData;
    AssemblyData(jsonData);

    CURL *p_curl = curl_easy_init();
    if (p_curl == NULL)
    {
        LOG(ERROR) << "curl_easy_init falid.";
        return false;
    }

    // 设置http发送的内容类型为JSON
    struct curl_slist *slist = NULL;
    slist = curl_slist_append(slist, "Content-Type:application/json;charset=UTF-8");
    curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, slist);

    // 禁用任何信号/警报处理程序
    curl_easy_setopt(p_curl, CURLOPT_NOSIGNAL, 1L);

    // 设置返回处理回调
    curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, &CurlCallbackWirteFunc);

    // 设置为非0表示本次操作为POST
    curl_easy_setopt(p_curl, CURLOPT_POST, 1);

    // 设置要POST的JSON数据
    curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, jsonData.size());

    // 设置机器人webhook地址
    curl_easy_setopt(p_curl, CURLOPT_URL, webhook.c_str());

    // 返回数据
    curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, retMsg);

    CURLcode errcode;
    errcode = curl_easy_perform(p_curl);
    if (CURLE_OK != errcode)
    {
        LOG(ERROR) << "CurlInterface.Post(), curl_easy_perform falid, errcode: " << errcode
                   << ", webhook = " << webhook
                   << ", \"jsonData\" : " << jsonData;

        curl_slist_free_all(slist);
        curl_easy_cleanup(p_curl);
        return false;
    }
    curl_slist_free_all(slist);
    curl_easy_cleanup(p_curl);
    return true;
}
