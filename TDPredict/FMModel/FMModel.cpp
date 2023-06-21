#include "FMModel.h"
#include "FMTrans.h"
#include <cmath>
#include "glog/logging.h"
#include "Common/Function.h"
using namespace TDPredict;

bool FMModel::Init(
    int factor,
    const std::string &model_file_path,
    const std::string &filter_file_path)
{
    return Update(factor, model_file_path, filter_file_path);
}

bool FMModel::Update(
    int factor,
    const std::string &model_file_path,
    const std::string &filter_file_path)
{
    std::lock_guard<std::mutex> lg(m_updateModelLock);

    // 缓存下标
    int data_idx = (m_modelDataIdx + 1) % 2;
    if (!m_modelData[data_idx].LoadModelFile(factor, model_file_path))
    {
        m_modelData[data_idx].Clear();
        return false;
    }

    if (!m_transData[data_idx].LoadTransFile(filter_file_path))
    {
        m_transData[data_idx].Clear();
        return false;
    }

    m_modelDataIdx = data_idx;
    return true;
}

bool FMModel::predict(RankItem &item) const noexcept
{
    if (item.spFeatureData == nullptr)
    {
        LOG(ERROR) << "predict() spFeatureData == nullptr";
        return false;
    }

    // 获取转换数据
    const int data_idx = m_modelDataIdx;
    const auto &trans_data = m_transData[data_idx];
    const auto &model_data = m_modelData[data_idx];
    const int factor = model_data.m_factor;

    // 计算item0 common部分
    item.sup_score = model_data.m_w0;
    item.sup_vec_sum.assign(factor, 0.0);
    item.sup_vec_sum_sqr.assign(factor, 0.0);
    predict_feature(item.spFeatureData->common_feature, trans_data.field_trans, data_idx, item);
    predict_feature(item.spFeatureData->rank_feature, trans_data.field_trans, data_idx, item);
    predict_score(item, data_idx);

    return true;
}

bool FMModel::predict(std::vector<RankItem> &vec_rank_item) const noexcept
{
    if (vec_rank_item.empty())
    {
        return true;
    }

    // 获取模型数据
    const int data_idx = m_modelDataIdx;
    const auto &trans_data = m_transData[data_idx];
    const auto &model_data = m_modelData[data_idx];
    const int factor = model_data.m_factor;

    auto &item0 = vec_rank_item[0];
    if (item0.spFeatureData == nullptr)
    {
        LOG(ERROR) << "predict() spFeatureData == nullptr";
        return false;
    }

    // 计算item0 common部分
    item0.sup_score = model_data.m_w0;
    item0.sup_vec_sum.assign(factor, 0.0);
    item0.sup_vec_sum_sqr.assign(factor, 0.0);
    predict_feature(item0.spFeatureData->common_feature, trans_data.field_trans, data_idx, item0);

    // item1~n common结果从item0复制
    for (int idx = 1, size = vec_rank_item.size(); idx != size; idx++)
    {
        vec_rank_item[idx].sup_score = item0.sup_score;
        vec_rank_item[idx].sup_vec_sum = item0.sup_vec_sum;
        vec_rank_item[idx].sup_vec_sum_sqr = item0.sup_vec_sum_sqr;
    }

    // 计算rank部分
    for (int idx = 0, size = vec_rank_item.size(); idx != size; idx++)
    {
        auto &item = vec_rank_item[idx];
        if (item.spFeatureData == nullptr)
        {
            LOG(ERROR) << "predict() spFeatureData == nullptr, idx = " << idx;
            return false;
        }

        predict_feature(item.spFeatureData->rank_feature, trans_data.field_trans, data_idx, item);
        predict_score(item, data_idx);
    }
    return true;
}

void FMModel::predict_feature(
    const TDPredict::FeatureItem &feature_item,
    const FMTransFormat &trans_format,
    const int data_idx,
    RankItem &item) const noexcept
{
    const auto &model_data = m_modelData[data_idx];
    const auto iter_w1_end = model_data.m_w1.end();
    const auto iter_w2_end = model_data.m_w2.end();
    const int factor = model_data.m_factor;

    for (const auto &pr_trans : trans_format)
    {
        const int trans_field = pr_trans.first;
        const int trans_count = pr_trans.second;

        const auto &field_item_list = feature_item[trans_field];
        if (trans_count == 0 || field_item_list.empty())
        {
            continue;
        }

        int use_count = std::min(trans_count, (int)field_item_list.size());
        for (int idx = 0; idx < use_count; idx++)
        {
            const auto &field_item = field_item_list[idx];
            const auto iter_w1 = model_data.m_w1.find(field_item.field_value.field_value());
            if (iter_w1 != iter_w1_end)
            {
                item.sup_score += iter_w1->second * field_item.weight;
            }

            const auto iter_w2 = model_data.m_w2.find(field_item.field_value.field_value());
            if (iter_w2 != iter_w2_end)
            {
                const auto &vec_w2 = iter_w2->second;
                for (int f_idx = 0; f_idx < factor; ++f_idx)
                {
                    score_type d = vec_w2[f_idx] * field_item.weight;
                    item.sup_vec_sum[f_idx] += d;
                    item.sup_vec_sum_sqr[f_idx] += (d * d);
                }
            }
        }
    }
}

void FMModel::predict_score(
    RankItem &item,
    const int data_idx) const noexcept
{
    const auto &model_data = m_modelData[data_idx];
    const int factor = model_data.m_factor;

    score_type result = item.sup_score;

    // 计算最终结果
    auto tmp_result = 0.0;
    for (int f_idx = 0; f_idx < factor; ++f_idx)
    {
        const auto &tmp_sum = item.sup_vec_sum[f_idx];
        tmp_result += (tmp_sum * tmp_sum - item.sup_vec_sum_sqr[f_idx]);
    }
    result += (tmp_result * 0.5);
    result = exp(-result);
    result = 1.0 / (1.0 + result);

    // 分数
    item.setModelScore(GetName(), result);
}
