#include "RankExpConfig.h"
#include <sstream>
#include "glog/logging.h"
#include "Common/Function.h"

#include "../FMModel/FMModel.h"
#include "../TFModel/TFModel.h"

using namespace TDPredict::Config;

bool RankExpConfig::Init(const std::string &json_file_path) noexcept
{
    return UpdateFile(json_file_path);
}

bool RankExpConfig::UpdateFile(const std::string &json_file_path) noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    LOG(INFO) << "TDPredict::RankExpConfig::UpdateFile"
              << ", json_file_path = " << json_file_path;
    if (json_file_path.empty())
    {
        m_lastErrorInfo = "TDPredict::RankExpConfig::UpdateFile json_file_path empty";
        LOG(ERROR) << m_lastErrorInfo;
        return false;
    }

    //读取配置
    if (RankExpConfig::Error::OK != this->LoadJson(json_file_path))
    {
        LOG(ERROR) << "TDPredict::RankExpConfig::UpdateFile LoadJson error"
                   << ", json_file_path = " << json_file_path;
        return false;
    }

    m_lastErrorInfo.clear();
    return true;
}

bool RankExpConfig::UpdateJson(const std::string &json_data) noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    if (json_data.empty())
    {
        m_lastErrorInfo = "TDPredict::RankExpConfig::UpdateJson() json_data empty.";
        LOG(ERROR) << m_lastErrorInfo;
        return false;
    }
    return RankExpConfig::Error::OK == DecodeJsonData(json_data);
}

const std::string RankExpConfig::GetConfigInfo() const noexcept
{
    static const std::string Platform_FMModel = "FMModel";
    static const std::string Platform_TensorFlow = "TensorFlow";

    std::lock_guard<std::mutex> lg(m_update_lock);
    std::stringstream ss;
    const auto &config_data = m_data[m_dataIdx];

    for (int idx = TDPredict::RankType::Begin; idx < TDPredict::RankType::End; idx++)
    {
        TDPredict::RankType rank_type = TDPredict::RankType(idx);
        ss << rank_type << ":\n";
        auto &exp2strategy = config_data.vecRankStrategy[rank_type];
        if (exp2strategy.empty())
        {
            ss << "\tEmpty Strategy List.\n";
            ss << std::endl;
            continue;
        }

        // 输出实验ID有序, 方便查看
        std::vector<int> vec_exp_id;
        {
            vec_exp_id.reserve(exp2strategy.size());
            for (auto &exp_pr : exp2strategy)
            {
                vec_exp_id.push_back(exp_pr.first);
            }
            std::sort(vec_exp_id.begin(), vec_exp_id.end());
        }

        for (const auto &exp_id : vec_exp_id)
        {
            const auto it = exp2strategy.find(exp_id);
            if (it == exp2strategy.end())
            {
                continue;
            }

            const auto &rank_strategy = it->second;

            ss << "\texp_id = " << exp_id << std::endl;
            if (!rank_strategy.BeforeStrategy.empty())
            {
                ss << "\trank_strategy.BeforeStrategy" << rank_strategy.BeforeStrategy << "\n";
            }
            if (!rank_strategy.ScoreStrategy.empty())
            {
                ss << "\trank_strategy.ScoreStrategy" << rank_strategy.ScoreStrategy << "\n";
            }
            if (!rank_strategy.EndStrategy.empty())
            {
                ss << "\trank_strategy.EndStrategy" << rank_strategy.EndStrategy << "\n";
            }
            if (rank_strategy.StrategyList.empty())
            {
                ss << "\trank_strategy.strategy_list is empty.\n";
                continue;
            }
            ss << "\trank_strategy.strategy_list:\n";
            const int strategy_list_size = rank_strategy.StrategyList.size();
            for (int strategy_idx = 0; strategy_idx != strategy_list_size; strategy_idx++)
            {
                const auto &strategy = rank_strategy.StrategyList[strategy_idx];
                ss << "\t    strategy_idx = " << strategy_idx << "\n";
                ss << "\t\tstrategy.Platform = " << strategy.Platform << "\n";
                ss << "\t\tstrategy.FeatureName = " << strategy.FeatureName << "\n";
                ss << "\t\tstrategy.ModelName = " << strategy.ModelName << "\n";
                ss << "\t\tstrategy.TruncCount = " << strategy.TruncCount << "\n";
                ss << "\t\tstrategy.CalcCount = " << strategy.CalcCount << "\n";
                if (!strategy.BeforeStrategy.empty())
                {
                    ss << "\t\tstrategy.BeforeStrategy = " << strategy.BeforeStrategy << "\n";
                }
                if (!strategy.ScoreStrategy.empty())
                {
                    ss << "\t\tstrategy.ScoreStrategy = " << strategy.ScoreStrategy << "\n";
                }
                if (!strategy.EndStrategy.empty())
                {
                    ss << "\t\tstrategy.EndStrategy = " << strategy.EndStrategy << "\n";
                }
            }
        }
        ss << std::endl;
    }
    return ss.str();
}

// 返回上次更新失败原因, 如果成功则返回空
const std::string RankExpConfig::GetLastErrorInfo() const noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    return m_lastErrorInfo;
}

const TDPredict::MapExp2RankStrategy *
RankExpConfig::GetExp2RankStrategy(const int32_t rank_type) const noexcept
{
    const auto &config_data = m_data[m_dataIdx];
    if (rank_type >= TDPredict::RankType::Begin && rank_type < TDPredict::RankType::End)
    {
        return &config_data.vecRankStrategy[rank_type];
    }
    return nullptr;
}

// 匹配粗排实验ID
const int RankExpConfig::MatchExpID(
    const int32_t rank_type,
    const std::vector<int> &vec_exp_id) const noexcept
{
    const auto map_exp2strategy = GetExp2RankStrategy(rank_type);
    if (map_exp2strategy == nullptr)
    {
        return 0;
    }

    // 匹配实验ID
    for (auto exp_id : vec_exp_id)
    {
        if (exp_id != 0)
        {
            auto itExp = map_exp2strategy->find(exp_id);
            if (itExp != map_exp2strategy->end())
            {
                return exp_id;
            }
        }
    }
    return 0;
}

// 匹配粗排实验策略列表
const TDPredict::RankStrategyConfigData &RankExpConfig::GetRankStrategy(
    const int32_t rank_type,
    const int exp_id) const noexcept
{
    static const TDPredict::RankStrategyConfigData empty;
    const auto map_exp2strategy = GetExp2RankStrategy(rank_type);
    if (map_exp2strategy == nullptr)
    {
        return empty;
    }

    // 匹配实验ID, 获取策略列表
    auto it = map_exp2strategy->find(exp_id);
    if (it == map_exp2strategy->end())
    {
        // 如果没有这个exp_id, 则使用默认exp_id -> 0
        it = map_exp2strategy->find(0);
    }
    if (it != map_exp2strategy->end())
    {
        return it->second;
    }
    return empty;
}

int32_t RankExpConfig::LoadJson(const std::string &json_file_path)
{
    std::string json_data;
    Common::FastReadFile(json_file_path, json_data, false);
    if (json_data.empty())
    {
        m_lastErrorInfo = "LoadJson() read json_file empty";
        LOG(ERROR) << m_lastErrorInfo;
        return RankExpConfig::Error::JsonDataEmpty;
    }
    return DecodeJsonData(json_data);
}

int32_t RankExpConfig::DecodeJsonData(const std::string &jsonData)
{
    rapidjson::Document doc;
    if (doc.Parse(jsonData.data()).HasParseError())
    {
        m_lastErrorInfo = "DecodeJsonData() Parse jsonData HasParseError, err = " + doc.GetParseError();
        LOG(ERROR) << m_lastErrorInfo;
        return RankExpConfig::Error::ParseJsonError;
    }

    // 缓存下标
    int data_idx = (m_dataIdx + 1) % 2;

    // 清理脏数据
    m_data[data_idx].clear();

    auto cfgErr = DecodePreRankExp(doc, data_idx);
    if (cfgErr != RankExpConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodePreRankExp Error.";
        return cfgErr;
    }

    cfgErr = DecodeRankExp(doc, data_idx);
    if (cfgErr != RankExpConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeRankExp Error.";
        return cfgErr;
    }

    cfgErr = DecodeReRankExp(doc, data_idx);
    if (cfgErr != RankExpConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeReRankExp Error.";
        return cfgErr;
    }

    m_dataIdx = data_idx;
    return RankExpConfig::Error::OK;
}

int32_t RankExpConfig::DecodePreRankExp(const rapidjson::Document &doc, const int data_idx)
{
    static const RankType rank_type = TDPredict::RankType::PreRank;
    if (!doc.HasMember("PreRankExp"))
    {
        return RankExpConfig::Error::OK;
    }

    if (!doc["PreRankExp"].IsArray())
    {
        LOG(ERROR) << "DecodePreRankExp() Parse PreRankExp Not Array";
        return RankExpConfig::Error::DecodePreRankExpError;
    }

    auto array = doc["PreRankExp"].GetArray();
    auto array_size = array.Size();
    m_data[data_idx].vecRankStrategy[rank_type].clear();
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        auto item = array[idx].GetObject();
        if (!item.HasMember("exp_id") || !item["exp_id"].IsInt())
        {
            LOG(ERROR) << "DecodePreRankExp() Parse jsonData"
                       << ", PreRankExp Not Find exp_id"
                       << ", idx = " << idx;
            return RankExpConfig::Error::DecodePreRankExpError;
        }
        int exp_id = item["exp_id"].GetInt();

        if (!item.HasMember("StrategyList") || !item["StrategyList"].IsArray())
        {
            LOG(ERROR) << "DecodePreRankExp() Parse jsonData"
                       << ", PreRankExp Not Find StrategyList"
                       << ", idx = " << idx;
            return RankExpConfig::Error::DecodePreRankExpError;
        }

        auto strategy_list = item["StrategyList"].GetArray();
        auto strategy_list_size = strategy_list.Size();
        auto &RankStrategy = m_data[data_idx].vecRankStrategy[rank_type][exp_id];
        RankStrategy.StrategyList.resize(strategy_list_size);
        for (rapidjson::SizeType strategy_idx = 0; strategy_idx < strategy_list_size; strategy_idx++)
        {
            auto strategy = strategy_list[strategy_idx].GetObject();
            auto &strategy_item = RankStrategy.StrategyList[strategy_idx];
            if (strategy.HasMember("Platform") && strategy["Platform"].IsString())
            {
                strategy_item.Platform = std::string(strategy["Platform"].GetString(),
                                                     strategy["Platform"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodePreRankExp() Parse jsonData"
                           << ", strategy Not Find Platform"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodePreRankExpError;
            }

            if (strategy.HasMember("FeatureName") && strategy["FeatureName"].IsString())
            {
                strategy_item.FeatureName = std::string(strategy["FeatureName"].GetString(),
                                                        strategy["FeatureName"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodePreRankExp() Parse jsonData"
                           << ", strategy Not Find FeatureName"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodePreRankExpError;
            }

            if (strategy.HasMember("ModelName") && strategy["ModelName"].IsString())
            {
                strategy_item.ModelName = std::string(strategy["ModelName"].GetString(),
                                                      strategy["ModelName"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodePreRankExp() Parse jsonData"
                           << ", strategy Not Find ModelName"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodePreRankExpError;
            }

            if (strategy.HasMember("TruncCount") && strategy["TruncCount"].IsInt())
            {
                strategy_item.TruncCount = strategy["TruncCount"].GetInt();
            }
            else
            {
                LOG(ERROR) << "DecodePreRankExp() Parse jsonData"
                           << ", strategy Not Find TruncCount"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodePreRankExpError;
            }

            if (strategy.HasMember("CalcCount") && strategy["CalcCount"].IsInt())
            {
                strategy_item.CalcCount = strategy["CalcCount"].GetInt();
            }
            else
            {
                // 如果不提供, 则计算数量与截断数量一致
                strategy_item.CalcCount = strategy_item.TruncCount;
            }

            // Model 新增 Before Socre End 三个策略
            if (strategy.HasMember("BeforeStrategy") && strategy["BeforeStrategy"].IsString())
            {
                strategy_item.BeforeStrategy = std::string(strategy["BeforeStrategy"].GetString(),
                                                           strategy["BeforeStrategy"].GetStringLength());
            }

            if (strategy.HasMember("ScoreStrategy") && strategy["ScoreStrategy"].IsString())
            {
                strategy_item.ScoreStrategy = std::string(strategy["ScoreStrategy"].GetString(),
                                                          strategy["ScoreStrategy"].GetStringLength());
            }

            if (strategy.HasMember("EndStrategy") && strategy["EndStrategy"].IsString())
            {
                strategy_item.EndStrategy = std::string(strategy["EndStrategy"].GetString(),
                                                        strategy["EndStrategy"].GetStringLength());
            }
        }

        // 排序层新增 Before Socre End 三个策略
        if (item.HasMember("BeforeStrategy") && item["BeforeStrategy"].IsString())
        {
            RankStrategy.BeforeStrategy = std::string(item["BeforeStrategy"].GetString(),
                                                      item["BeforeStrategy"].GetStringLength());
        }

        if (item.HasMember("ScoreStrategy") && item["ScoreStrategy"].IsString())
        {
            RankStrategy.ScoreStrategy = std::string(item["ScoreStrategy"].GetString(),
                                                     item["ScoreStrategy"].GetStringLength());
        }

        if (item.HasMember("EndStrategy") && item["EndStrategy"].IsString())
        {
            RankStrategy.EndStrategy = std::string(item["EndStrategy"].GetString(),
                                                   item["EndStrategy"].GetStringLength());
        }
    }
    return RankExpConfig::Error::OK;
}

int32_t RankExpConfig::DecodeRankExp(const rapidjson::Document &doc, const int data_idx)
{
    static const RankType rank_type = TDPredict::RankType::Rank;

    if (!doc.HasMember("RankExp"))
    {
        return RankExpConfig::Error::OK;
    }

    if (!doc["RankExp"].IsArray())
    {
        LOG(ERROR) << "DecodeRankExp() Parse RankExp Not Array";
        return RankExpConfig::Error::DecodeRankExpError;
    }

    auto array = doc["RankExp"].GetArray();
    auto array_size = array.Size();
    m_data[data_idx].vecRankStrategy[rank_type].clear();
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        auto item = array[idx].GetObject();
        if (!item.HasMember("exp_id") || !item["exp_id"].IsInt())
        {
            LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                       << ", RankExp Not Find exp_id"
                       << ", idx = " << idx;
            return RankExpConfig::Error::DecodeRankExpError;
        }
        if (!item.HasMember("StrategyList") || !item["StrategyList"].IsArray())
        {
            LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                       << ", RankExp Not Find StrategyList"
                       << ", idx = " << idx;
            return RankExpConfig::Error::DecodeRankExpError;
        }

        int exp_id = item["exp_id"].GetInt();
        auto strategy_list = item["StrategyList"].GetArray();
        auto strategy_list_size = strategy_list.Size();
        auto &RankStrategy = m_data[data_idx].vecRankStrategy[rank_type][exp_id];
        RankStrategy.StrategyList.resize(strategy_list_size);
        for (rapidjson::SizeType strategy_idx = 0; strategy_idx < strategy_list_size; strategy_idx++)
        {
            auto strategy = strategy_list[strategy_idx].GetObject();
            auto &strategy_item = RankStrategy.StrategyList[strategy_idx];
            if (strategy.HasMember("Platform") && strategy["Platform"].IsString())
            {
                strategy_item.Platform = std::string(strategy["Platform"].GetString(),
                                                     strategy["Platform"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                           << ", strategy Not Find Platform"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeRankExpError;
            }

            if (strategy.HasMember("FeatureName") && strategy["FeatureName"].IsString())
            {
                strategy_item.FeatureName = std::string(strategy["FeatureName"].GetString(),
                                                        strategy["FeatureName"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                           << ", strategy Not Find FeatureName"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeRankExpError;
            }

            if (strategy.HasMember("ModelName") && strategy["ModelName"].IsString())
            {
                strategy_item.ModelName = std::string(strategy["ModelName"].GetString(),
                                                      strategy["ModelName"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                           << ", strategy Not Find ModelName"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeRankExpError;
            }

            if (strategy.HasMember("TruncCount") && strategy["TruncCount"].IsInt())
            {
                strategy_item.TruncCount = strategy["TruncCount"].GetInt();
            }
            else
            {
                LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                           << ", strategy Not Find TruncCount"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeRankExpError;
            }

            if (strategy.HasMember("CalcCount") && strategy["CalcCount"].IsInt())
            {
                strategy_item.CalcCount = strategy["CalcCount"].GetInt();
            }
            else
            {
                // 如果不提供, 则计算数量与截断数量一致
                strategy_item.CalcCount = strategy_item.TruncCount;
            }

            // Model 新增 Before Socre End 三个策略
            if (strategy.HasMember("BeforeStrategy") && strategy["BeforeStrategy"].IsString())
            {
                strategy_item.BeforeStrategy = std::string(strategy["BeforeStrategy"].GetString(),
                                                           strategy["BeforeStrategy"].GetStringLength());
            }

            if (strategy.HasMember("ScoreStrategy") && strategy["ScoreStrategy"].IsString())
            {
                strategy_item.ScoreStrategy = std::string(strategy["ScoreStrategy"].GetString(),
                                                          strategy["ScoreStrategy"].GetStringLength());
            }

            if (strategy.HasMember("EndStrategy") && strategy["EndStrategy"].IsString())
            {
                strategy_item.EndStrategy = std::string(strategy["EndStrategy"].GetString(),
                                                        strategy["EndStrategy"].GetStringLength());
            }
        }

        // 排序层新增 Before Socre End 三个策略
        if (item.HasMember("BeforeStrategy") && item["BeforeStrategy"].IsString())
        {
            RankStrategy.BeforeStrategy = std::string(item["BeforeStrategy"].GetString(),
                                                      item["BeforeStrategy"].GetStringLength());
        }

        if (item.HasMember("ScoreStrategy") && item["ScoreStrategy"].IsString())
        {
            RankStrategy.ScoreStrategy = std::string(item["ScoreStrategy"].GetString(),
                                                     item["ScoreStrategy"].GetStringLength());
        }

        if (item.HasMember("EndStrategy") && item["EndStrategy"].IsString())
        {
            RankStrategy.EndStrategy = std::string(item["EndStrategy"].GetString(),
                                                   item["EndStrategy"].GetStringLength());
        }
    }
    return RankExpConfig::Error::OK;
}

int32_t RankExpConfig::DecodeReRankExp(const rapidjson::Document &doc, const int data_idx)
{
    static const RankType rank_type = TDPredict::RankType::ReRank;

    if (!doc.HasMember("ReRankExp"))
    {
        return RankExpConfig::Error::OK;
    }

    if (!doc["ReRankExp"].IsArray())
    {
        LOG(ERROR) << "DecodeReRankExp() Parse ReRankExp Not Array";
        return RankExpConfig::Error::DecodeReRankExpError;
    }

    auto array = doc["ReRankExp"].GetArray();
    auto array_size = array.Size();
    m_data[data_idx].vecRankStrategy[rank_type].clear();
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        auto item = array[idx].GetObject();
        if (!item.HasMember("exp_id") || !item["exp_id"].IsInt())
        {
            LOG(ERROR) << "DecodeReRankExp() Parse jsonData"
                       << ", ReRankExp Not Find exp_id"
                       << ", idx = " << idx;
            return RankExpConfig::Error::DecodeReRankExpError;
        }
        if (!item.HasMember("StrategyList") || !item["StrategyList"].IsArray())
        {
            LOG(ERROR) << "DecodeReRankExp() Parse jsonData"
                       << ", ReRankExp Not Find StrategyList"
                       << ", idx = " << idx;
            return RankExpConfig::Error::DecodeReRankExpError;
        }

        int exp_id = item["exp_id"].GetInt();
        auto strategy_list = item["StrategyList"].GetArray();
        auto strategy_list_size = strategy_list.Size();
        auto &RankStrategy = m_data[data_idx].vecRankStrategy[rank_type][exp_id];
        RankStrategy.StrategyList.resize(strategy_list_size);
        for (rapidjson::SizeType strategy_idx = 0; strategy_idx < strategy_list_size; strategy_idx++)
        {
            auto strategy = strategy_list[strategy_idx].GetObject();
            auto &strategy_item = RankStrategy.StrategyList[strategy_idx];
            if (strategy.HasMember("Platform") && strategy["Platform"].IsString())
            {
                strategy_item.Platform = std::string(strategy["Platform"].GetString(),
                                                     strategy["Platform"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodeReRankExp() Parse jsonData"
                           << ", strategy Not Find Platform"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeReRankExpError;
            }

            if (strategy.HasMember("FeatureName") && strategy["FeatureName"].IsString())
            {
                strategy_item.FeatureName = std::string(strategy["FeatureName"].GetString(),
                                                        strategy["FeatureName"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodeRankExp() Parse jsonData"
                           << ", strategy Not Find FeatureName"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeRankExpError;
            }

            if (strategy.HasMember("ModelName") && strategy["ModelName"].IsString())
            {
                strategy_item.ModelName = std::string(strategy["ModelName"].GetString(),
                                                      strategy["ModelName"].GetStringLength());
            }
            else
            {
                LOG(ERROR) << "DecodeReRankExp() Parse jsonData"
                           << ", strategy Not Find ModelName"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeReRankExpError;
            }

            if (strategy.HasMember("TruncCount") && strategy["TruncCount"].IsInt())
            {
                strategy_item.TruncCount = strategy["TruncCount"].GetInt();
            }
            else
            {
                LOG(ERROR) << "DecodeReRankExp() Parse jsonData"
                           << ", strategy Not Find TruncCount"
                           << ", strategy_idx = " << strategy_idx;
                return RankExpConfig::Error::DecodeReRankExpError;
            }

            if (strategy.HasMember("CalcCount") && strategy["CalcCount"].IsInt())
            {
                strategy_item.CalcCount = strategy["CalcCount"].GetInt();
            }
            else
            {
                // 如果不提供, 则计算数量与截断数量一致
                strategy_item.CalcCount = strategy_item.TruncCount;
            }

            // Model 新增 Before Socre End 三个策略
            if (strategy.HasMember("BeforeStrategy") && strategy["BeforeStrategy"].IsString())
            {
                strategy_item.BeforeStrategy = std::string(strategy["BeforeStrategy"].GetString(),
                                                           strategy["BeforeStrategy"].GetStringLength());
            }

            if (strategy.HasMember("ScoreStrategy") && strategy["ScoreStrategy"].IsString())
            {
                strategy_item.ScoreStrategy = std::string(strategy["ScoreStrategy"].GetString(),
                                                          strategy["ScoreStrategy"].GetStringLength());
            }

            if (strategy.HasMember("EndStrategy") && strategy["EndStrategy"].IsString())
            {
                strategy_item.EndStrategy = std::string(strategy["EndStrategy"].GetString(),
                                                        strategy["EndStrategy"].GetStringLength());
            }
        }

        // 排序层新增 Before Socre End 三个策略
        if (item.HasMember("BeforeStrategy") && item["BeforeStrategy"].IsString())
        {
            RankStrategy.BeforeStrategy = std::string(item["BeforeStrategy"].GetString(),
                                                      item["BeforeStrategy"].GetStringLength());
        }

        if (item.HasMember("ScoreStrategy") && item["ScoreStrategy"].IsString())
        {
            RankStrategy.ScoreStrategy = std::string(item["ScoreStrategy"].GetString(),
                                                     item["ScoreStrategy"].GetStringLength());
        }

        if (item.HasMember("EndStrategy") && item["EndStrategy"].IsString())
        {
            RankStrategy.EndStrategy = std::string(item["EndStrategy"].GetString(),
                                                   item["EndStrategy"].GetStringLength());
        }
    }
    return RankExpConfig::Error::OK;
}
