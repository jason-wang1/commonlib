#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include "FieldValue.h"
#include "Eigen/Dense"

namespace TDPredict
{
    using score_type = float;
    using emb_any_type = Eigen::Matrix<score_type, 1, -1>;
    inline void vector2emb(std::vector<score_type> &data, emb_any_type &emb) noexcept
    {
        emb = Eigen::Map<emb_any_type, Eigen::Unaligned>(data.data(), data.size());
    }

    // TensorFlow 模型调用方案
    enum TFModelMethodType
    {
        Restful = 1,
        GRPC = 2,
    };

    // TensorFlow 模型用户类型
    enum TFModelUserType
    {
        Default = 0, // 默认, 老用户
        T_0 = 1,     // T+0新用户
    };

    // 排序层类型
    enum RankType : int32_t
    {
        Invalid = 0,     // 无效值
        Begin = 1,       // 起始游标
        PreRank = Begin, // 粗排层
        Rank,            // 精排层
        ReRank,          // 重排层
        Count,           // 排序层数量
        End = Count,     // 结束游标
    };

    inline std::ostream &operator<<(std::ostream &out, RankType rank_type)
    {
        switch (rank_type)
        {
        case RankType::PreRank:
            out << "PreRank";
            break;
        case RankType::Rank:
            out << "Rank";
            break;
        case RankType::ReRank:
            out << "ReRank";
            break;
        default:
            break;
        }
        out << "(" << static_cast<std::underlying_type<RankType>::type>(rank_type) << ")";
        return out;
    }

    // 规定特征域的最大值
    inline constexpr int FIELD_MAX = 256;

    struct FieldItem
    {
        FieldValue field_value = 0;
        score_type weight = 0.0; // 权重
        // FieldItem(const long fv, score_type w) : field_value(fv), weight(w) {}
        FieldItem(const int f, const int v, score_type w) : field_value(f, v), weight(w) {}
    };

    using FieldItemList = std::vector<FieldItem>;
    using FeatureItem = std::vector<FieldItemList>;
    struct FeatureData
    {
        FeatureItem common_feature; // 公共特征
        FeatureItem rank_feature;   // 排序特征

        FeatureData()
        {
            common_feature.resize(FIELD_MAX);
            rank_feature.resize(FIELD_MAX);
        }

        FeatureData(FeatureData &&f)
        {
            this->common_feature = std::move(f.common_feature);
            this->rank_feature = std::move(f.rank_feature);
        }

        FeatureData &operator=(FeatureData &&f)
        {
            this->common_feature = std::move(f.common_feature);
            this->rank_feature = std::move(f.rank_feature);
            return *this;
        }
    };

    // 计算单元
    struct RankItem
    {
        // 模型 - 分数
        std::unordered_map<RankType, score_type> type2model_score;    // 模型层对应分数
        std::unordered_map<std::string, score_type> name2model_score; // 模型名称对应分数

        // 仅用于模型计算完毕后, 排序比较
        score_type cmp_score = 0;

        // 特征
        TFModelUserType user_type = TFModelUserType::Default; // 用户类型
        std::shared_ptr<FeatureData> spFeatureData = nullptr;
        std::unordered_map<std::string, std::shared_ptr<FeatureData>> name2spFeatureData;

        // FM辅助计算
        score_type sup_score = 0;
        std::vector<score_type> sup_vec_sum;
        std::vector<score_type> sup_vec_sum_sqr;

        // 其他参数
        std::unordered_map<std::string, long> long_params;
        std::unordered_map<std::string, double> double_params;
        std::unordered_map<std::string, std::string> string_params;

        RankItem() {}

        RankItem(RankItem &&item)
        {
            this->cmp_score = item.cmp_score;
            this->type2model_score = std::move(item.type2model_score);
            this->name2model_score = std::move(item.name2model_score);
            this->user_type = item.user_type;
            this->spFeatureData = std::move(item.spFeatureData);
            this->name2spFeatureData = std::move(item.name2spFeatureData);
            this->sup_score = item.sup_score;
            this->sup_vec_sum = std::move(item.sup_vec_sum);
            this->sup_vec_sum_sqr = std::move(item.sup_vec_sum_sqr);
            this->long_params = std::move(item.long_params);
            this->double_params = std::move(item.double_params);
            this->string_params = std::move(item.string_params);
        };

        RankItem &operator=(RankItem &&item)
        {
            this->cmp_score = item.cmp_score;
            this->type2model_score = std::move(item.type2model_score);
            this->name2model_score = std::move(item.name2model_score);
            this->user_type = item.user_type;
            this->spFeatureData = std::move(item.spFeatureData);
            this->name2spFeatureData = std::move(item.name2spFeatureData);
            this->sup_score = item.sup_score;
            this->sup_vec_sum = std::move(item.sup_vec_sum);
            this->sup_vec_sum_sqr = std::move(item.sup_vec_sum_sqr);
            this->long_params = std::move(item.long_params);
            this->double_params = std::move(item.double_params);
            this->string_params = std::move(item.string_params);
            return *this;
        }

    public:
        const score_type getRankScore(const RankType rank_type) const noexcept
        {
            const auto it = type2model_score.find(rank_type);
            if (it != type2model_score.end())
            {
                return it->second;
            }
            return 0.0;
        };

        const bool setRankScore(const RankType rank_type, score_type value) noexcept
        {
            type2model_score[rank_type] = value;
            return true;
        };

        const score_type getModelScore(const std::string &model_name) const noexcept
        {
            const auto it = name2model_score.find(model_name);
            if (it != name2model_score.end())
            {
                return it->second;
            }
            return 0.0;
        }

        const bool setModelScore(const std::string &model_name, score_type value) noexcept
        {
            name2model_score[model_name] = value;
            return true;
        }

    public:
        bool getParam(const std::string &key, long &value) const noexcept
        {
            const auto it = long_params.find(key);
            if (it != long_params.end())
            {
                value = it->second;
                return true;
            }
            value = 0;
            return false;
        }

        bool getParam(const std::string &key, double &value) const noexcept
        {
            const auto it = double_params.find(key);
            if (it != double_params.end())
            {
                value = it->second;
                return true;
            }
            value = 0;
            return false;
        }

        bool getParam(const std::string &key, std::string &value) const noexcept
        {
            const auto it = string_params.find(key);
            if (it != string_params.end())
            {
                value = it->second;
                return true;
            }
            value.clear();
            return false;
        }

        void setParam(const std::string &key, const long value) noexcept
        {
            long_params[key] = value;
        }

        void setParam(const std::string &key, const double value) noexcept
        {
            double_params[key] = value;
        }

        void setParam(const std::string &key, const std::string &value) noexcept
        {
            string_params[key] = value;
        }

        void setParam(const std::string &key, std::string &&value) noexcept
        {
            string_params[key] = std::move(value);
        }
    };

    // 模型公共接口
    class ModelInterface
    {
    public:
        // 模型名称
        virtual std::string GetName() const noexcept
        {
            return m_ModelName;
        }

    public:
        // 预测函数
        virtual bool predict(RankItem &item) const noexcept = 0;
        virtual bool predict(std::vector<RankItem> &vec_item) const noexcept = 0;

    public:
        ModelInterface(const std::string &model_name)
        {
            m_ModelName = model_name;
        }
        virtual ~ModelInterface() {}

    private:
        std::string m_ModelName;
    };
} // namespace TDRank
