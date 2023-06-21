#pragma once
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include "Common/Singleton.h"
#include "rapidjson/document.h"

#include "ConfigData.h"
#include "../Interface/ConfigInterface.h"

namespace TDPredict
{
    class SENet;
    namespace Config
    {
        class SENetConfig
            : public ConfigInterface,
              public Singleton<SENetConfig>
        {
        public:
            virtual bool Init(const std::string &json_file_path) noexcept override;
            virtual bool UpdateFile(const std::string &json_file_path) noexcept override;
            virtual bool UpdateJson(const std::string &json_data) noexcept override;
            virtual const std::string GetConfigInfo() const noexcept override;

        public:
            void SetDefaultRestfulAddrList(const std::vector<std::string> &vecAddrList) noexcept
            {
                std::lock_guard<std::mutex> lg(m_update_lock);
                m_vecDefaultRestfulAddrList = vecAddrList;
            }

            void SetDefaultGRPCAddrList(const std::vector<std::string> &vecAddrList) noexcept
            {
                std::lock_guard<std::mutex> lg(m_update_lock);
                m_vecDefaultGRPCAddrList = vecAddrList;
            }

            // 返回上次更新失败原因, 如果成功则返回空
            const std::string GetLastErrorInfo() const noexcept;

            // 获取排序模型
            const std::shared_ptr<SENet> GetSENet(
                const std::string &model_name) const noexcept;

        protected:
            typedef enum SENetConfigError
            {
                OK = 0,
                JsonDataEmpty = 1,
                ParseJsonError,
                DecodeTFSModelFolderError,
                DecodeDefaultSENetAddrListError,
                DecodeSENetError,
                UpdateModelError,
            } Error;

            int32_t LoadJson(const std::string &filename);
            int32_t DecodeJsonData(const std::string &jsonData);
            int32_t DecodeTFSModelFolder(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeSENet(const rapidjson::Document &doc, const int data_idx);
            int32_t UpdateModel(const int dataIdx);

        public:
            SENetConfig(token) { m_dataIdx = 0; }
            virtual ~SENetConfig() {}
            SENetConfig(SENetConfig &) = delete;
            SENetConfig &operator=(const SENetConfig &) = delete;

        private:
            mutable std::mutex m_update_lock; // 更新锁
            std::atomic<int> m_dataIdx;
            SENetConfigData m_data[2];
            std::string m_lastErrorInfo; // 最后一次更新的具体失败原因
            std::vector<std::string> m_vecDefaultRestfulAddrList;
            std::vector<std::string> m_vecDefaultGRPCAddrList;
        };
    }
}