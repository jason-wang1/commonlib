#include "SENet.h"
#include "TFSCommon.hpp"
using namespace TDPredict;

// 更新配置
bool SENet::Update(
    const std::vector<std::string> &vec_tfs_addrs,
    const std::string &signature_name,
    const int timeout,
    const std::string &trans_file_path,
    const bool use_dropout_keep,
    const int version,
    const int emb_size,
    const TFModelMethodType request_method,
    const std::string &output_name) noexcept
{
    std::lock_guard<std::mutex> lg(m_updateLock);
    const int old_data_idx = m_dataIdx;
    const int new_data_idx = (m_dataIdx + 1) % 2;
    {
        // 释放旧对象
        m_spTFModelBase[new_data_idx] = nullptr;
        m_spTFModelGrpc[new_data_idx] = nullptr;
        m_spTFTransData[new_data_idx] = nullptr;
        m_spEmbCacheData[new_data_idx] = nullptr;

        m_RequestMethod[new_data_idx] = request_method;
        if (TFModelMethodType::Restful == request_method)
        {
            // 创建新的对象
            m_spTFModelBase[new_data_idx] = std::make_shared<TFModelBase>(
                vec_tfs_addrs, signature_name,
                m_ModelName, timeout, use_dropout_keep, version);
        }
        else if (TFModelMethodType::GRPC == request_method)
        {
            // 仅在GRPC模式下使用 output_name 字段
            m_spTFModelGrpc[new_data_idx] = std::make_shared<TFModelGrpc>(
                vec_tfs_addrs, signature_name, m_ModelName,
                timeout, use_dropout_keep, version, output_name, false);
        }
        else
        {
            return false;
        }

        m_spTFTransData[new_data_idx] = std::make_shared<TFTransData>();
        if (m_spTFTransData[new_data_idx] == nullptr ||
            !m_spTFTransData[new_data_idx]->LoadTransFile(trans_file_path))
        {
            m_spTFTransData[new_data_idx] = nullptr;
            return false;
        }

        m_spEmbCacheData[new_data_idx] = std::make_shared<std::vector<std::shared_ptr<EmbCacheData>>>();
        if (m_spEmbCacheData[new_data_idx] != nullptr)
        {
            m_spEmbCacheData[new_data_idx]->reserve(CacheBucketCount);
            for (int idx = 0; idx < CacheBucketCount; idx++)
            {
                m_spEmbCacheData[new_data_idx]->emplace_back(std::make_shared<EmbCacheData>());
            }
        }

        m_EmbSize[new_data_idx] = emb_size;
    }
    m_dataIdx = new_data_idx;

    // 旧的ModelBase可以直接释放, 当引用计数为0时自动析构
    m_spTFModelBase[old_data_idx] = nullptr;
    m_spTFModelGrpc[old_data_idx] = nullptr;
    m_spTFTransData[old_data_idx] = nullptr;
    m_spEmbCacheData[old_data_idx] = nullptr;

    return true;
}

// 批量预测函数
bool SENet::predict(
    const std::vector<long> &vec_ids,
    const std::vector<TDPredict::TFSInputData> &vec_tfs_inputs,
    std::vector<emb_any_type> &vec_result_emb) const noexcept
{
    if (vec_tfs_inputs.empty())
    {
        return true;
    }

    const int data_idx = m_dataIdx;
    const auto request_method = m_RequestMethod[data_idx];
    const int emb_size = m_EmbSize[data_idx];

    const int vec_item_size = vec_tfs_inputs.size();
    vec_result_emb.resize(vec_item_size);
    if (TFModelMethodType::Restful == request_method)
    {
        // 获取ModelBase
        auto spTFModelBase = m_spTFModelBase[data_idx];
        if (spTFModelBase == nullptr)
        {
            LOG(ERROR) << "predict() spTFModelBase is nullptr, data_idx = " << data_idx;
            return false;
        }

        // 调用ModelBase predict
        std::vector<std::string> vec_tfs_results(vec_item_size);
        spTFModelBase->predict_base(vec_tfs_inputs, vec_tfs_results);

        std::vector<score_type> result_vector;
        for (int idx = 0; idx != vec_item_size; idx++)
        {
            auto &tfs_result = vec_tfs_results[idx];
            if (tfs_result.empty())
            {
                continue;
            }

            result_vector.clear();
            if (!get_tfs_result(tfs_result, result_vector))
            {
                LOG(ERROR) << "predict() get_tfs_result failed"
                           << ", model_name = " << m_ModelName
                           << ", tfs_result = " << tfs_result;
                return false;
            }

#ifdef DEBUG
            if (idx < 3)
            {
                // 输出物料的 请求数据 & 计算结果
                LOG(INFO) << "[DEBUG] predict() tfs_post idx = " << idx
                          << ", id = " << vec_ids[idx]
                          << ", model_name = " << m_ModelName
                          << ", post_msg = " << spTFModelBase->make_tfs_post_msg(vec_tfs_inputs[idx])
                          << ", result_vector = " << Common::to_json_array(result_vector);
            }
#endif

            const int result_vec_size = result_vector.size();
            if (result_vec_size != emb_size)
            {
                LOG(ERROR) << "predict() check emb_size failed"
                           << ", result_vec_size = " << result_vec_size
                           << ", emb_size = " << emb_size;
                return false;
            }

            vector2emb(result_vector, vec_result_emb[idx]);
        }
        return true;
    }
    else if (TFModelMethodType::GRPC == request_method)
    {
        // 获取ModelBase
        auto spTFModelGRPC = m_spTFModelGrpc[data_idx];
        if (spTFModelGRPC == nullptr)
        {
            LOG(ERROR) << "predict() spTFModelGRPC is nullptr, data_idx = " << data_idx;
            return false;
        }

        // 调用ModelBase predict
        return spTFModelGRPC->predict_emb(vec_tfs_inputs, vec_result_emb);
    }
    else
    {
        LOG(ERROR) << "predict() request_method not match, request_method = " << request_method;
        return false;
    }
}

// 预测函数
bool SENet::predict(
    const long id,
    const FeatureItem &feature_item,
    emb_any_type &result_emb) const noexcept
{
    // 转换数据
    const int data_idx = m_dataIdx;
    const auto spTransData = m_spTFTransData[data_idx];
    if (spTransData == nullptr)
    {
        LOG(ERROR) << "predict() spTransData is nullptr, data_idx = " << data_idx;
        return false;
    }

    // 转换为tfs数据
    std::vector<TDPredict::TFSInputData> vec_tfs_inputs(1);
    TDPredict::TFSInputData &tfs_input = vec_tfs_inputs[0];
    tfs_input.index.resize(spTransData->all_field_count, 0);
    tfs_input.value.resize(spTransData->all_field_count, 0.0f);
    TransTFSFeature(feature_item, *spTransData, tfs_input);

    // 调用实际计算函数
    std::vector<long> vec_ids(1);
    vec_ids[0] = id;
    std::vector<emb_any_type> vec_result_emb(1);
    if (predict(vec_ids, vec_tfs_inputs, vec_result_emb))
    {
        result_emb = vec_result_emb[0];
        return true;
    }
    return false;
}

// 批量预测函数
bool SENet::predict(
    const std::vector<long> &vec_ids,
    const std::vector<FeatureItem> &vec_feature_item,
    std::vector<emb_any_type> &vec_result_emb) const noexcept
{
    if (vec_feature_item.empty())
    {
        return true;
    }

    // 转换数据
    const int data_idx = m_dataIdx;
    const auto spTransData = m_spTFTransData[data_idx];
    if (spTransData == nullptr)
    {
        LOG(ERROR) << "predict() spTransData is nullptr, data_idx = " << data_idx;
        return false;
    }

    // 转换为tfs数据
    const int vec_feature_item_size = vec_feature_item.size();
    std::vector<TDPredict::TFSInputData> vec_tfs_inputs(vec_feature_item_size);
    for (int idx = 0; idx != vec_feature_item_size; idx++)
    {
        auto &tfs_input = vec_tfs_inputs[idx];
        tfs_input.index.resize(spTransData->all_field_count, 0);
        tfs_input.value.resize(spTransData->all_field_count, 0.0f);
        TransTFSFeature(vec_feature_item[idx], *spTransData, tfs_input);
    }
    return predict(vec_ids, vec_tfs_inputs, vec_result_emb);
}

// 批量预测排序特征函数 - 使用缓存
bool SENet::predict_rank_cache(
    const std::vector<RankItem> &vec_rank_item,
    std::vector<emb_any_type> &vec_result_emb) noexcept
{
    vec_result_emb.clear();
    if (vec_rank_item.empty())
    {
        return true;
    }

    const int data_idx = m_dataIdx;

    // 缓存
    auto spEmbCacheData = m_spEmbCacheData[data_idx];
    if (spEmbCacheData == nullptr)
    {
        LOG(ERROR) << "predict() spEmbCacheData is nullptr, data_idx = " << data_idx;
        return false;
    }

    // 转换数据
    const auto spTransData = m_spTFTransData[data_idx];
    if (spTransData == nullptr)
    {
        LOG(ERROR) << "predict() spTransData is nullptr, data_idx = " << data_idx;
        return false;
    }

    const int vec_rank_item_size = vec_rank_item.size();
    vec_result_emb.resize(vec_rank_item_size);

    // 先查缓存
    std::vector<int> vec_miss_cache_idx;
    vec_miss_cache_idx.reserve(vec_rank_item_size);
    for (int idx = 0; idx != vec_rank_item_size; idx++)
    {
        long map_id = 0;
        if (!vec_rank_item[idx].getParam("map_id", map_id))
        {
            // 没有地图ID字段, 则用不了缓存
            vec_miss_cache_idx.emplace_back(idx);
            continue;
        }

        // 根据地图ID查询读写锁缓存
        auto spCache = spEmbCacheData->at(map_id % CacheBucketCount);
        if (!spCache->GetRWCacheData(map_id, vec_result_emb[idx]))
        {
            vec_miss_cache_idx.emplace_back(idx);
        }
    }

    // 缓存内没有查到的则通过SENet计算得到, 同时写入缓存
    if (vec_miss_cache_idx.empty())
    {
        return true;
    }

    // 转换为tfs数据
    const int miss_cache_size = vec_miss_cache_idx.size();
    std::vector<TDPredict::TFSInputData> vec_tfs_inputs(miss_cache_size);
    std::vector<long> vec_ids(miss_cache_size, 0);
    for (int idx = 0; idx != miss_cache_size; idx++)
    {
        const int raw_idx = vec_miss_cache_idx[idx];
        const auto &feature_item = vec_rank_item[raw_idx].spFeatureData->rank_feature;

        vec_rank_item[raw_idx].getParam("map_id", vec_ids[idx]);

        auto &tfs_input = vec_tfs_inputs[idx];
        tfs_input.index.resize(spTransData->all_field_count, 0);
        tfs_input.value.resize(spTransData->all_field_count, 0.0f);
        TransTFSFeature(feature_item, *spTransData, tfs_input);
    }

    std::vector<emb_any_type> miss_cache_emb(miss_cache_size);
    if (predict(vec_ids, vec_tfs_inputs, miss_cache_emb))
    {
        std::vector<std::unordered_map<Key, Value>> vec_rw_cache_buffer;
        vec_rw_cache_buffer.resize(CacheBucketCount);

        for (int idx = 0; idx != miss_cache_size; idx++)
        {
            const int raw_idx = vec_miss_cache_idx[idx];
            vec_result_emb[raw_idx] = std::move(miss_cache_emb[idx]);

            // 读取map_id, 有map_id则可以使用缓存
            long map_id = 0;
            if (vec_rank_item[raw_idx].getParam("map_id", map_id))
            {
                const int bucket = map_id % CacheBucketCount;
                vec_rw_cache_buffer[bucket][map_id] = vec_result_emb[raw_idx];
            }
        }

        // 计算成功, 将新的Emb写入缓存
        for (int idx = 0; idx < CacheBucketCount; idx++)
        {
            if (!vec_rw_cache_buffer[idx].empty())
            {
                auto spCache = spEmbCacheData->at(idx);
                spCache->AddRWBufCacheData(vec_rw_cache_buffer[idx]);
            }
        }
        return true;
    }
    return false;
}
