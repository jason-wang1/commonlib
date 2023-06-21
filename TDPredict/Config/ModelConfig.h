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
    class ModelInterface; // 排序模型
    class SENet;          // 神经网络模型
    namespace Config
    {
        class ModelConfig
            : public ConfigInterface,
              public Singleton<ModelConfig>
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

            // 获取模型
            const std::shared_ptr<TDPredict::ModelInterface> GetModel(
                const std::string &platform,
                const std::string &model_name) const noexcept;

            // 获取SENet模型
            const std::shared_ptr<SENet> GetSENet(
                const std::string &model_name) const noexcept;

        protected:
            typedef enum ModelConfigError
            {
                OK = 0,
                JsonDataEmpty = 1,
                ParseJsonError,
                DecodeFMModelFolderError,
                DecodeTFSModelFolderError,
                DecodeFMModelError,
                DecodeTFModelError,
                DecodeSENetModelError,
                UpdateModelError,
            } Error;

            int32_t LoadJson(const std::string &filename);
            int32_t DecodeJsonData(const std::string &jsonData);
            int32_t DecodeFMModelFolder(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeTFSModelFolder(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeFMModel(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeTFModel(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeSENetModel(const rapidjson::Document &doc, const int data_idx);
            int32_t UpdateModel(const int dataIdx);

        public:
            ModelConfig(token) { m_dataIdx = 0; }
            virtual ~ModelConfig() {}
            ModelConfig(ModelConfig &) = delete;
            ModelConfig &operator=(const ModelConfig &) = delete;

        private:
            mutable std::mutex m_update_lock; // 更新锁
            std::atomic<int> m_dataIdx;
            ModelConfigData m_data[2];
            std::string m_lastErrorInfo;                          // 最后一次更新的具体失败原因
            std::vector<std::string> m_vecDefaultRestfulAddrList; // 默认Restful协议调用地址
            std::vector<std::string> m_vecDefaultGRPCAddrList;    // 默认GRPC协议调用地址
        };

    } // namespace Config
} // namespace TDPredict
