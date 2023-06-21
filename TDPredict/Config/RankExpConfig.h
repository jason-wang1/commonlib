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
    class RankModelInterface;
    namespace Config
    {
        class RankExpConfig
            : public ConfigInterface,
              public Singleton<RankExpConfig>
        {
        public:
            virtual bool Init(const std::string &json_file_path) noexcept override;
            virtual bool UpdateFile(const std::string &json_file_path) noexcept override;
            virtual bool UpdateJson(const std::string &json_data) noexcept override;
            virtual const std::string GetConfigInfo() const noexcept override;

        public:
            // 返回上次更新失败原因, 如果成功则返回空
            const std::string GetLastErrorInfo() const noexcept;

            // 匹配实验ID
            const int MatchExpID(const int32_t rank_type, const std::vector<int> &vec_exp_id) const noexcept;

            // 匹配实验排序策略
            const TDPredict::RankStrategyConfigData &GetRankStrategy(const int32_t rank_type, const int exp_id) const noexcept;

        protected:
            typedef enum RankExpConfigError
            {
                OK = 0,
                JsonDataEmpty = 1,
                ParseJsonError,
                DecodePreRankExpError,
                DecodeRankExpError,
                DecodeReRankExpError,
            } Error;

            int32_t LoadJson(const std::string &filename);
            int32_t DecodeJsonData(const std::string &jsonData);
            int32_t DecodePreRankExp(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeRankExp(const rapidjson::Document &doc, const int data_idx);
            int32_t DecodeReRankExp(const rapidjson::Document &doc, const int data_idx);

            // 匹配类型对应实验Map
            const TDPredict::MapExp2RankStrategy *GetExp2RankStrategy(const int32_t rank_type) const noexcept;

        public:
            RankExpConfig(token) { m_dataIdx = 0; }
            virtual ~RankExpConfig() {}
            RankExpConfig(RankExpConfig &) = delete;
            RankExpConfig &operator=(const RankExpConfig &) = delete;

        private:
            mutable std::mutex m_update_lock; // 更新锁
            std::atomic<int> m_dataIdx;
            RankExpConfigData m_data[2];
            std::string m_lastErrorInfo; // 最后一次更新的具体失败原因
        };

    } // namespace Config
} // namespace TDPredict
