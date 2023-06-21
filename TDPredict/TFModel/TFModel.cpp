#include "TFModel.h"
#include "TFSCommon.hpp"
using namespace TDPredict;

// 更新配置
bool TFModel::Update(
    const std::vector<std::string> &vec_tfs_addrs,
    const std::string &signature_name,
    const int timeout,
    const std::string &trans_file_path,
    const bool use_dropout_keep,
    const int version,
    const TFModelMethodType request_method,
    const std::string &output_name,
    const bool use_user_type) noexcept
{
    std::lock_guard<std::mutex> lg(m_updateLock);
    int data_idx = (m_dataIdx + 1) % 2;

    m_RequestMethod[data_idx] = request_method;
    if (TFModelMethodType::Restful == request_method)
    {
        m_spTFModelBase[data_idx] = std::make_shared<TFModelBase>(
            vec_tfs_addrs, signature_name, GetName(),
            timeout, use_dropout_keep, version);
    }
    else if (TFModelMethodType::GRPC == request_method)
    {
        // 仅在GRPC模式下使用 output_name 字段
        m_spTFModelGrpc[data_idx] = std::make_shared<TFModelGrpc>(
            vec_tfs_addrs, signature_name, GetName(),
            timeout, use_dropout_keep, version, output_name, use_user_type);
    }
    else
    {
        return false;
    }

    m_spTFTransData[data_idx] = std::make_shared<TFTransData>();
    if (m_spTFTransData[data_idx] == nullptr ||
        !m_spTFTransData[data_idx]->LoadTransFile(trans_file_path))
    {
        m_spTFTransData[data_idx] = nullptr;
        return false;
    }

    m_dataIdx = data_idx;
    return true;
}

// 预测函数
bool TFModel::predict(RankItem &item) const noexcept
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
bool TFModel::predict(std::vector<RankItem> &vec_rank_item) const noexcept
{
    if (vec_rank_item.empty())
    {
        return true;
    }

    // 转换数据
    const int data_idx = m_dataIdx;
    const auto request_method = m_RequestMethod[data_idx];

    const auto spTransData = m_spTFTransData[data_idx];
    if (spTransData == nullptr)
    {
        LOG(ERROR) << "predict() spTransData is nullptr, data_idx = " << data_idx;
        return false;
    }

    // 转换为tfs数据
    const int vec_rank_item_size = vec_rank_item.size();
    std::vector<TDPredict::TFSInputData> vec_tfs_inputs(vec_rank_item_size);
    GenerateTFSInput(vec_rank_item, *spTransData, vec_tfs_inputs);

    if (TFModelMethodType::Restful == request_method)
    {
        auto &spTFModelBase = m_spTFModelBase[data_idx];
        if (spTFModelBase == nullptr)
        {
            LOG(ERROR) << "predict() spTFModelBase == nullptr, data_idx = " << data_idx;
            return false;
        }

        std::vector<std::string> vec_tfs_results(vec_rank_item_size);
        spTFModelBase->predict_base(vec_tfs_inputs, vec_tfs_results);

        for (int idx = 0; idx != vec_rank_item_size; idx++)
        {
            auto &tfs_result = vec_tfs_results[idx];
            if (tfs_result.empty())
            {
                continue;
            }

            TDPredict::score_type tfs_result_score = 0.0;
            if (!get_tfs_result(tfs_result, tfs_result_score))
            {
                LOG(ERROR) << "predict() get_tfs_result failed"
                           << ", idx = " << idx
                           << ", model_name = " << GetName()
                           << ", tfs_result = " << tfs_result;
                continue;
            }

            // 将分数写入模型名称
            vec_rank_item[idx].setModelScore(GetName(), tfs_result_score);
        }
        return true;
    }
    else if (TFModelMethodType::GRPC == request_method)
    {
        auto &spTFModelGrpc = m_spTFModelGrpc[data_idx];
        if (spTFModelGrpc == nullptr)
        {
            LOG(ERROR) << "predict() spTFModelGrpc == nullptr, data_idx = " << data_idx;
            return false;
        }

        std::vector<TDPredict::score_type> vec_tfs_score(vec_rank_item_size);
        if (spTFModelGrpc->predict_score(vec_tfs_inputs, vec_tfs_score))
        {
            for (int idx = 0; idx < vec_rank_item_size; idx++)
            {
                // 将分数写入模型名称
                vec_rank_item[idx].setModelScore(GetName(), vec_tfs_score[idx]);
            }
            return true;
        }
        return false;
    }
    else
    {
        LOG(ERROR) << "predict() request_method not match, request_method = " << request_method;
        return false;
    }
}

bool TFModel::GenerateTFSInput(
    const std::vector<RankItem> &vec_rank_item,
    const TFTransData &trans_data,
    std::vector<TDPredict::TFSInputData> &vec_tfs_inputs) const noexcept
{
    if (vec_rank_item.empty())
    {
        return true;
    }

    TDPredict::TFSInputData tfs_common_input;
    // 计算common部分
    {
        tfs_common_input.index.resize(trans_data.all_field_count, 0);
        tfs_common_input.value.resize(trans_data.all_field_count, 0.0f);

        auto &item_0 = vec_rank_item[0];
        if (item_0.spFeatureData == nullptr)
        {
            LOG(ERROR) << "GenerateTFSInput() item_0.spFeatureData == nullptr";
            return false;
        }
        TransTFSFeature(item_0.spFeatureData->common_feature, trans_data, tfs_common_input);
    }

    // 计算rank部分
    const size_t vec_rank_item_size = vec_rank_item.size();
    for (size_t idx = 0; idx < vec_rank_item_size; idx++)
    {
        auto &tfs_input = vec_tfs_inputs[idx];
        tfs_input.index = tfs_common_input.index;
        tfs_input.value = tfs_common_input.value;

        auto &item = vec_rank_item[idx];
        if (item.spFeatureData == nullptr)
        {
            LOG(ERROR) << "GenerateTFSInput() item.spFeatureData == nullptr, idx = " << idx;
            return false;
        }
        TransTFSFeature(item.spFeatureData->rank_feature, trans_data, tfs_input);

        tfs_input.user_type = item.user_type;
    }
    return true;
}
