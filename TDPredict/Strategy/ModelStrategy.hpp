#pragma once
#include "../Config/ConfigData.h"

namespace TDPredict
{
    namespace Strategy
    {
        inline void TFModel_Pairwise(
            const TDPredict::RankType &rank_type,
            const TDPredict::StrategyConfigData &strategy,
            std::vector<TDPredict::RankItem> &vec_rank_item)
        {
            for (auto &item : vec_rank_item)
            {
                item.cmp_score = item.getModelScore(strategy.ModelName);
            }

            // 使用模型排序分数进行排序比较
            static const auto &cmp = [](const RankItem &a, const RankItem &b)
            { return a.cmp_score > b.cmp_score; };
            std::sort(vec_rank_item.begin(), vec_rank_item.end(), cmp);

            constexpr score_type basicRatio = 10.0;
            constexpr score_type minScore = 1.0;
            constexpr score_type maxScore = 10.0;

            // 从后往前, 取第一个大于0的差值作为增量
            const auto last_score = vec_rank_item.back().getModelScore(strategy.ModelName);
            score_type incr_score = 0.0;
            for (auto rIt = vec_rank_item.rbegin(); rIt != vec_rank_item.rend(); rIt++)
            {
                auto score = rIt->getModelScore(strategy.ModelName);
                incr_score = score - last_score;
                if (incr_score > 1e-6)
                {
                    break;
                }
            }

            if (incr_score < 1e-6)
            {
                incr_score = 1.0;
            }

            // #ifdef DEBUG
            // for (auto &item : vec_rank_item)
            // {
            //     auto score = item.getModelScore(strategy.ModelName);
            //     LOG(INFO) << "[DEBUG] before_score = " << score;
            // }
            // LOG(INFO) << "[DEBUG] incr_score = " << incr_score;
            // #endif

            score_type fScoreRatio = 1.0;
            for (int idx = 0, size = vec_rank_item.size(); idx != size; idx++)
            {
                auto &item = vec_rank_item[idx];
                auto score = item.getModelScore(strategy.ModelName);
                auto new_score = score - last_score + incr_score;
                if (new_score < 1e-6)
                {
                    return;
                }

                if (idx == 0)
                {
                    while (true)
                    {
                        if (new_score - minScore < 1e-6)
                        {
                            new_score *= basicRatio;
                            fScoreRatio *= basicRatio;
                        }
                        else if (new_score - maxScore > 1e-6)
                        {
                            new_score /= basicRatio;
                            fScoreRatio /= basicRatio;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    new_score *= fScoreRatio;
                }
                item.setModelScore(strategy.ModelName, new_score);
            }

            // #ifdef DEBUG
            // LOG(INFO) << "[DEBUG] fScoreRatio = " << fScoreRatio;
            // for (auto &item : vec_rank_item)
            // {
            //     auto score = item.getModelScore(strategy.ModelName);
            //     LOG(INFO) << "[DEBUG] end_score = " << score;
            // }
            // #endif
        }
    }
}
