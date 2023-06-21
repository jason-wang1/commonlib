#pragma once
#include "../Interface/RankInterface.h"
#include "../Interface/ModelInterface.h"
#include "../Interface/FeatureInterface.h"
#include <any>
#include <unordered_set>
#include <unordered_map>
#include "PrometheusClient/PrometheusClient.h"

namespace TDPredict
{
    struct StrategyConfigData;
    class RankCalc final : public RankInterface
    {
    public:
        // 计算接口
        virtual void Calc(
            const std::vector<int> &vec_exp_id,
            std::vector<RankItem> &vec_rank_item,
            std::unordered_map<TDPredict::RankType, int> &exp_id_mapping) const noexcept override;

    public:
        bool SetUUID(const std::string &uuid);
        bool SetFeatureFactory(const FeatureFactoryPtr &ptr);
        bool SetRankLogCount(const int RankLogCount);
        bool SetRankLogFeature(const bool log = true);
        bool SetUserRegisterTimestamp(const long timestamp);
        bool SetNewUserWhitelist(const std::unordered_set<long> &Whitelist);
        bool SetPrometheusReport(const PrometheusReportPtr &spReport);
        bool SetInitData(const std::any init_data);

    private:
        bool CalcSteategy(
            const TDPredict::RankType &rank_type,
            const TDPredict::StrategyConfigData &strategy,
            std::vector<RankItem> &vec_rank_item) const noexcept;

        bool PrintRankLog(
            const std::vector<int> &vec_exp_id,
            const std::vector<RankItem> &vec_rank_item) const noexcept;

    private:
        std::string m_uuid;                                 // 请求唯一标识
        FeatureFactoryPtr m_factory = nullptr;              // 特征工厂
        std::any m_init_data;                               // 特征初始化数据
        int m_RankLogCount = 0;                             // 打印日志数量
        bool m_RankLogFeature = false;                      // 是否打印FM特征
        long m_UserRegisterTimestamp = 0;                   // 用户注册时间戳
        std::unordered_set<long> m_NewUserWhitelist;        // 新用户白名单
        PrometheusReportPtr m_spPrometheusReport = nullptr; // 普罗米修斯上报
    };
} // namespace TDPredict
