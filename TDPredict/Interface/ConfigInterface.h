#pragma once
#include <string>
#include <unordered_set>

namespace TDPredict
{
    // ConfigInterface 配置公共接口
    class ConfigInterface
    {
        virtual bool Init(const std::string &json_file_path) noexcept = 0;
        virtual bool UpdateFile(const std::string &json_file_path) noexcept = 0;
        virtual bool UpdateJson(const std::string &json_data) noexcept = 0;
        virtual const std::string GetConfigInfo() const noexcept = 0;
    };

} // namespace TDPredict