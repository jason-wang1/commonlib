#include "RankCalc.h"
#include <algorithm>
#include "glog/logging.h"
#include "Common/Function.h"

#include "../Config/ModelConfig.h"
#include "../Config/RankExpConfig.h"
#include "../Strategy/ModelStrategy.hpp"

using namespace TDPredict::Config;

void TDPredict::RankCalc::Calc(
    const std::vector<int> &vec_exp_id,
    std::vector<RankItem> &vec_rank_item,
    std::unordered_map<TDPredict::RankType, int> &exp_id_mapping) const noexcept
{
    static const std::vector<TDPredict::RankType> vec_rank_type = {
        TDPredict::RankType::PreRank,
        TDPredict::RankType::Rank,
        TDPredict::RankType::ReRank,
    };

    const auto &rank_exp_config = RankExpConfig::GetInstance();
    for (const auto &rank_type : vec_rank_type)
    {
        if (m_spPrometheusReport != nullptr)
        {
            std::stringstream ss;
            ss << rank_type;
            m_spPrometheusReport->start(ss.str());
        }

        const int &exp_id = rank_exp_config->MatchExpID(rank_type, vec_exp_id);
        exp_id_mapping.insert(std::make_pair(rank_type, exp_id));

        const auto &rankStrategy = rank_exp_config->GetRankStrategy(rank_type, exp_id);

        // 执行前置策略
        // 通过条件判断, 决定是否启用该排序层
        // rankStrategy.BeforeStrategy;

        int run_strategy_count = 0;
        const auto &strategyList = rankStrategy.StrategyList;
        for (const auto &strategy : strategyList)
        {
#ifdef DEBUG
            const auto begin_time = Common::get_ms_time();
#endif

            if (m_spPrometheusReport != nullptr)
            {
                m_spPrometheusReport->start(strategy.ModelName);
            }

            // 策略调用成功失败, 只告知策略是否执行
            // 比如因为前置策略判断(用户是否为新用户), 导致策略未执行, 也会返回 false
            if (CalcSteategy(rank_type, strategy, vec_rank_item))
            {
                run_strategy_count += 1;
            }

            if (m_spPrometheusReport != nullptr)
            {
                m_spPrometheusReport->end(0);
            }

#ifdef DEBUG
            const auto end_time = Common::get_ms_time();
            LOG(INFO) << "[DEBUG] Calc() CalcSteategyTime"
                      << ", uuid = " << m_uuid
                      << ", rank_type = " << rank_type
                      << ", vec_rank_item_size = " << vec_rank_item.size()
                      << ", strategy.Platform = " << strategy.Platform
                      << ", strategy.FeatureName = " << strategy.FeatureName
                      << ", strategy.ModelName = " << strategy.ModelName
                      << ", strategy.CalcCount = " << strategy.CalcCount
                      << ", strategy.TruncCount = " << strategy.TruncCount
                      << ", time = " << end_time - begin_time;
#endif
        }

        // 仅当该层有成功执行策略时, 执行分数策略
        if (run_strategy_count > 0)
        {
            // 执行分数策略
            // rankStrategy.ScoreStrategy;

            // 对每个模型模型, 执行分数加权策略, 得到该层最终分数
            for (auto &item : vec_rank_item)
            {
                // 默认使用加和策略
                item.cmp_score = 0.0;
                for (auto &strategy : strategyList)
                {
                    item.cmp_score += item.getModelScore(strategy.ModelName);
                }
                item.setRankScore(rank_type, item.cmp_score);
            }

            // 排序层分数有改变, 执行结束策略
            // 使用模型分数进行排序, 交换元素位置
            static const auto &cmp = [](const RankItem &a, const RankItem &b)
            { return a.cmp_score > b.cmp_score; };
            std::sort(vec_rank_item.begin(), vec_rank_item.end(), cmp);
        }

        // 执行后置策略
        // rankStrategy.EndStrategy;

        if (m_spPrometheusReport != nullptr)
        {
            m_spPrometheusReport->end(0);
        }
    }

    // 打印日志
    if (m_RankLogCount)
    {
        PrintRankLog(vec_exp_id, vec_rank_item);
    }
}

bool TDPredict::RankCalc::SetUUID(const std::string &uuid)
{
    this->m_uuid = uuid;
    return true;
}

bool TDPredict::RankCalc::SetFeatureFactory(const FeatureFactoryPtr &spFactory)
{
    if (spFactory != nullptr)
    {
        this->m_factory = spFactory;
        return true;
    }
    return false;
}

bool TDPredict::RankCalc::SetRankLogCount(const int RankLogCount)
{
    this->m_RankLogCount = RankLogCount;
    return true;
}

bool TDPredict::RankCalc::SetRankLogFeature(const bool log)
{
    this->m_RankLogFeature = log;
    return true;
}

bool TDPredict::RankCalc::SetUserRegisterTimestamp(const long timestamp)
{
    this->m_UserRegisterTimestamp = timestamp;
    return true;
}

bool TDPredict::RankCalc::SetNewUserWhitelist(const std::unordered_set<long> &Whitelist)
{
    m_NewUserWhitelist = Whitelist;
    return true;
}

bool TDPredict::RankCalc::SetPrometheusReport(const PrometheusReportPtr &spReport)
{
    if (spReport != nullptr)
    {
        this->m_spPrometheusReport = spReport;
        return true;
    }
    return false;
}

bool TDPredict::RankCalc::SetInitData(const std::any init_data)
{
    this->m_init_data = init_data;
    return true;
}

bool TDPredict::RankCalc::CalcSteategy(
    const TDPredict::RankType &rank_type,
    const TDPredict::StrategyConfigData &strategy,
    std::vector<RankItem> &vec_rank_item) const noexcept
{
    if (vec_rank_item.empty())
    {
        return true;
    }

    auto &item_0 = vec_rank_item[0];
    long user_id = 0;
    if (!item_0.getParam("user_id", user_id))
    {
        LOG(ERROR) << "CalcSteategy() getParam user_id Failed";
        return false;
    }

    // 前置策略, 加特征 Or 判断计算条件
    {
        // 判断新用户, 如果不是新用户, 不执行策略, 返回false
        if (strategy.BeforeStrategy == "IsNewUser")
        {
            // 新用户判断条件为注册时间距离当前时间24h内
            constexpr long time_24h = 86400;
            if (Common::get_timestamp() - m_UserRegisterTimestamp > time_24h &&
                m_NewUserWhitelist.find(user_id) == m_NewUserWhitelist.end())
            {
#ifdef DEBUG
                LOG(INFO) << "[DEBUG] CalcSteategy() BeforeStrategy == IsNewUser, not new user"
                          << ", user_id = " << user_id;
#endif
                return false;
            }
        }
        else if (strategy.BeforeStrategy == "NotPredict")
        {
            return false;
        }
    }

    // 获取模型
    const auto &model_config = ModelConfig::GetInstance();
    const auto spModel = model_config->GetModel(strategy.Platform, strategy.ModelName);
    if (spModel == nullptr)
    {
        LOG(ERROR) << "CalcSteategy() GetModel Failed"
                   << ", strategy.Platform = " << strategy.Platform
                   << ", strategy.ModelName = " << strategy.ModelName;
        return false;
    }

    // 截断数量, 如果为0标识不截断
    if (strategy.TruncCount > 0)
    {
        const int vec_rank_item_size = vec_rank_item.size();
        const int min_count = std::min(strategy.TruncCount, vec_rank_item_size);
        vec_rank_item.resize(min_count);
    }

    // 计算数量, 如果为0则全部计算
    std::vector<RankItem> vec_back_item; // 备份列表
    if (strategy.CalcCount > 0)
    {
        const int vec_rank_item_size = vec_rank_item.size();
        const int min_count = std::min(strategy.CalcCount, vec_rank_item_size);

        // 不计算的数据放到备份列表
        vec_back_item.reserve(vec_rank_item_size);
        for (int i = min_count; i < vec_rank_item_size; i++)
        {
            vec_back_item.emplace_back(std::move(vec_rank_item[i]));
        }

        // resize截断计算部分
        vec_rank_item.resize(min_count);
    }

    // 拼接特征
    auto it = item_0.name2spFeatureData.find(strategy.FeatureName);
    if (it != item_0.name2spFeatureData.end())
    {
        // 特征存在, 则直接指向它
        for (int idx = 0, size = vec_rank_item.size(); idx < size; idx++)
        {
            auto &rank_item = vec_rank_item[idx];
            rank_item.spFeatureData = rank_item.name2spFeatureData[strategy.FeatureName];
        }
    }
    else
    {
        // 特征不存在, 则需要新增特征
        if (m_factory == nullptr)
        {
            LOG(ERROR) << "CalcSteategy() m_factory is nullptr";
            return false;
        }

        // 获取特征模型
        auto spFeature = m_factory->Create(strategy.FeatureName);
        if (spFeature == nullptr)
        {
            LOG(ERROR) << "CalcSteategy() factory Create spFeature is nullptr"
                       << ", strategy.FeatureName = " << strategy.FeatureName;
            return false;
        }
        if (!spFeature->SetInitData(m_init_data))
        {
            LOG(ERROR) << "CalcSteategy() spFeature->SetInitData Failed";
            return false;
        }

        // 新建一个FeatureData
        item_0.spFeatureData = std::make_shared<FeatureData>();
        if (item_0.spFeatureData == nullptr)
        {
            LOG(ERROR) << "CalcSteategy() make_shared FeatureData Failed";
            return false;
        }

        // 提前获取common特征, 所有item的common特征一样
        spFeature->AnalyseCommonFeature(item_0);
        spFeature->AnalyseRankFeature(item_0);
        item_0.name2spFeatureData[strategy.FeatureName] = item_0.spFeatureData;

        for (int idx = 1, size = vec_rank_item.size(); idx < size; idx++)
        {
            auto &rank_item = vec_rank_item[idx];
            rank_item.spFeatureData = std::make_shared<FeatureData>();
            if (rank_item.spFeatureData == nullptr)
            {
                LOG(ERROR) << "CalcSteategy() make_shared FeatureData Failed";
                return false;
            }

            rank_item.spFeatureData->common_feature = item_0.spFeatureData->common_feature;
            spFeature->AnalyseRankFeature(rank_item);
            rank_item.name2spFeatureData[strategy.FeatureName] = rank_item.spFeatureData;
        }
    }

    // 打分计算成功, 再调用排序;
    // 如果计算失败, 则不调用排序, 相当于本次计算无效.
    if (spModel->predict(vec_rank_item))
    {
        // 分数策略
        if (strategy.ScoreStrategy == "TFModel_Pairwise")
        {
            TDPredict::Strategy::TFModel_Pairwise(rank_type, strategy, vec_rank_item);
        }
        else
        {
            for (auto &item : vec_rank_item)
            {
                item.cmp_score = item.getModelScore(strategy.ModelName);
            }

            // 使用模型排序分数进行排序比较
            static const auto &cmp = [](const RankItem &a, const RankItem &b)
            { return a.cmp_score > b.cmp_score; };
            std::sort(vec_rank_item.begin(), vec_rank_item.end(), cmp);
        }
    }

    // 结束策略
    // strategy.EndStrategy;

    // 将不计算部分数据放回
    const int vec_back_item_size = vec_back_item.size();
    for (int i = 0; i < vec_back_item_size; i++)
    {
        vec_rank_item.emplace_back(std::move(vec_back_item[i]));
    }
    return true;
}

bool TDPredict::RankCalc::PrintRankLog(
    const std::vector<int> &vec_exp_id,
    const std::vector<RankItem> &vec_rank_item) const noexcept
{
    std::string str_exp_list;
    {
        bool ss_flag = false;
        std::stringstream ss_exp_list;
        ss_exp_list << "[";
        for (const auto &exp_id : vec_exp_id)
        {
            if (ss_flag)
            {
                ss_exp_list << ",";
            }
            ss_exp_list << exp_id;
            ss_flag = true;
        }
        ss_exp_list << "]";
        str_exp_list = ss_exp_list.str();
    }

    int log_count = 0;
    for (const auto &item : vec_rank_item)
    {
        log_count += 1;

        std::string strParam;
        {
            strParam.reserve(200);
            for (const auto &pr : item.long_params)
            {
                if (!strParam.empty())
                {
                    strParam += ", ";
                }
                strParam += pr.first + " = " + std::to_string(pr.second);
            }
            for (const auto &pr : item.double_params)
            {
                if (!strParam.empty())
                {
                    strParam += ", ";
                }
                strParam += pr.first + " = " + std::to_string(pr.second);
            }
            for (const auto &pr : item.string_params)
            {
                if (!strParam.empty())
                {
                    strParam += ", ";
                }
                strParam += pr.first + " = " + pr.second;
            }
        }

        std::string str_rank_type_score;
        {
            bool ss_flag = false;
            std::stringstream ss_rank_type_score;
            ss_rank_type_score << "[";
            for (const auto &item_score : item.type2model_score)
            {
                if (ss_flag)
                {
                    ss_rank_type_score << ",";
                }
                ss_rank_type_score << "{\"type\":\"" << item_score.first << "\",\"score\":" << item_score.second << "}";
                ss_flag = true;
            }
            ss_rank_type_score << "]";
            str_rank_type_score = ss_rank_type_score.str();
        }

        std::string str_model_score;
        {
            bool ss_flag = false;
            std::stringstream ss_model_score;
            ss_model_score << "[";
            for (const auto &item_score : item.name2model_score)
            {
                if (ss_flag)
                {
                    ss_model_score << ",";
                }
                ss_model_score << "{\"model\":" << item_score.first << ",\"score\":" << item_score.second << "}";
                ss_flag = true;
            }
            ss_model_score << "]";
            str_model_score = ss_model_score.str();
        }

        std::stringstream ss_model_feature;
        if (m_RankLogFeature)
        {
            for (auto &pr : item.name2spFeatureData)
            {
                std::string strFeature;
                strFeature.reserve(200);
                const auto &feature_name = pr.first;
                const auto &feature_data = pr.second;
                if (feature_data != nullptr)
                {
                    const auto &common_feature = feature_data->common_feature;
                    for (const auto &feature_list : common_feature)
                    {
                        for (const auto &field_item : feature_list)
                        {
                            strFeature += std::to_string(field_item.field_value.field_value()) + ":" + std::to_string(field_item.weight) + " ";
                        }
                    }

                    const auto &rank_feature = feature_data->rank_feature;
                    for (const auto &feature_list : rank_feature)
                    {
                        for (const auto &field_item : feature_list)
                        {
                            strFeature += std::to_string(field_item.field_value.field_value()) + ":" + std::to_string(field_item.weight) + " ";
                        }
                    }
                }
                ss_model_feature << ", " << feature_name << " = " << strFeature;
            }
        }

        LOG(INFO) << "TDPredict::RankCalc::Calc() RankLog"
                  << ", uuid = " << m_uuid
                  << ", " << strParam
                  << ", exp_list = " << str_exp_list
                  << ", user_type = " << item.user_type            // 用户类型
                  << ", rank_type_score = " << str_rank_type_score // 排序层-分数
                  << ", model_score = " << str_model_score         // 模型名称-分数
                  << ss_model_feature.str();

        if (log_count > m_RankLogCount)
        {
            break;
        }
    }
    return true;
}