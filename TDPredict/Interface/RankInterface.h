#pragma once
#include "ModelInterface.h"

namespace TDPredict
{
    // RankInterface 排序计算公共接口
    // 1. 匹配实验ID, 获取排序策略
    // 2. 执行排序策略
    // 2-1. 获取排序模型
    // 2-2. 截断计算数量
    // 2-3. 计算打分
    // 2-4. 按照分数sort排序
    // 2-4. 单策略计算完毕
    // 3. 截断 & 返回结果
    class RankInterface
    {
    public:
        // 计算接口
        /**
         * @param vec_exp_id 实验列表
         * @param vec_rank_item 排序单元
         * @param exp_id_mapping 排序层匹配实验
         **/
        virtual void Calc(
            const std::vector<int> &vec_exp_id,
            std::vector<RankItem> &vec_rank_item,
            std::unordered_map<TDPredict::RankType, int> &exp_id_mapping) const noexcept = 0;
    };
} // namespace TDPredict