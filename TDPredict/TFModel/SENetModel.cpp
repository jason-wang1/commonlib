#include "SENetModel.h"
#include "SENet.h"
#include "../Config/SENetConfig.h"
using namespace TDPredict;

// 更新配置
bool SENetModel::Update(
    const std::string &user_model_name,
    const std::string &item_model_name) noexcept
{
    std::lock_guard<std::mutex> lg(m_updateLock);
    int data_idx = (m_dataIdx + 1) % 2;
    {
        auto config = TDPredict::Config::SENetConfig::GetInstance();
        m_spUserSENet[data_idx] = config->GetSENet(user_model_name);
        if (m_spUserSENet[data_idx] == nullptr)
        {
            return false;
        }

        m_spItemSENet[data_idx] = config->GetSENet(item_model_name);
        if (m_spItemSENet[data_idx] == nullptr)
        {
            return false;
        }

        if (m_spItemSENet[data_idx]->GetEmbSize() !=
            m_spUserSENet[data_idx]->GetEmbSize())
        {
            return false;
        }
    }
    m_dataIdx = data_idx;
    return true;
}

// 预测函数
bool SENetModel::predict(RankItem &item) const noexcept
{
    std::vector<RankItem> vec_rank_item(1);
    vec_rank_item[0] = std::move(item);
    if (predict(vec_rank_item))
    {
        item = std::move(vec_rank_item[0]);
        return true;
    }
    return false;
}

// 批量预测函数
bool SENetModel::predict(std::vector<RankItem> &vec_rank_item) const noexcept
{
    const int data_idx = m_dataIdx;
    auto userSENet = m_spUserSENet[data_idx];
    auto itemSENet = m_spItemSENet[data_idx];
    if (userSENet == nullptr || itemSENet == nullptr)
    {
        LOG(ERROR) << "predict() userSENet == nullptr || userSENet == nullptr";
        return false;
    }

    if (vec_rank_item.empty())
    {
        return true;
    }

    // 1. 计算用户Emb
    emb_any_type user_emb;
    {
        auto &item_0 = vec_rank_item[0];
        if (item_0.spFeatureData == nullptr)
        {
            LOG(ERROR) << "GenerateTFSInput() item_0.spFeatureData == nullptr";
            return false;
        }

        long uin = 0;
        item_0.getParam("uin", uin);
        if (!userSENet->predict(uin, item_0.spFeatureData->common_feature, user_emb))
        {
            return false;
        }
        if (user_emb.size() != userSENet->GetEmbSize())
        {
            return false;
        }
    }

    // 2. 计算物料Emb
    const int vec_rank_item_size = vec_rank_item.size();
    std::vector<emb_any_type> vec_item_emb(vec_rank_item_size);
    if (!itemSENet->predict_rank_cache(vec_rank_item, vec_item_emb))
    {
        return false;
    }

    // 3. 点积得到结果
    for (int idx = 0; idx != vec_rank_item_size; idx++)
    {
        const auto &item_emb = vec_item_emb[idx];
        if (user_emb.size() == item_emb.size())
        {
            // 将分数写入模型名称
            auto score = user_emb.dot(item_emb);
            vec_rank_item[idx].setModelScore(GetName(), score);
        }
    }
    return true;
}
