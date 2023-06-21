#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "curl/curl.h"
#include "glog/logging.h"

class LarkMessage final
{
    using TextList = std::vector<std::pair<std::string, std::string>>;

public:
    LarkMessage() {}
    ~LarkMessage() {}
    LarkMessage(const LarkMessage &) = delete;
    LarkMessage &operator=(const LarkMessage &) = delete;

public:
    void SetOther(const std::string &other);

    void SetTitle(const std::string &title);

    void AddText(const std::string &key, const std::string &text);

    bool Send(const std::string &webhook, std::string &retMsg);

protected:
    void AssemblyData(std::string &outData);

private:
    std::string m_other;
    std::string m_title;
    TextList m_text_list;
};
