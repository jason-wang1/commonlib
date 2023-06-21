#pragma once
#include <memory>
#include <atomic>
#include <mutex>
#include "Common/CommonCache.h"

#include "TFTrans.h"
#include "TFModelBase.h"
#include "TFModelGrpc.h"
#include "../Interface/ModelInterface.h"

namespace TDPredict
{

    class SENet final
    {
        using Key = long;
        using Value = emb_any_type;
        using EmbCacheData = HashCache<Key, Value>;
        const static int CacheBucketCount = 100;

    public:
        SENet(const std::string &model_name)
            : m_ModelName(model_name) {}
        ~SENet() {}

        // 更新配置
        bool Update(
            const std::vector<std::string> &vec_tfs_addrs,
            const std::string &signature_name,
            const int timeout,
            const std::string &trans_file_path,
            const bool use_dropout_keep,
            const int version,
            const int emb_size,
            const TFModelMethodType request_method,
            const std::string &output_name) noexcept;

        const int GetEmbSize() const noexcept
        {
            return m_EmbSize[m_dataIdx];
        }

    public:
        // 批量预测函数
        bool predict(
            const std::vector<long> &vec_ids,
            const std::vector<TDPredict::TFSInputData> &vec_tfs_inputs,
            std::vector<emb_any_type> &vec_result_emb) const noexcept;

        // 预测函数
        bool predict(
            const long id,
            const FeatureItem &feature_item,
            emb_any_type &result_emb) const noexcept;

        // 批量预测函数
        bool predict(
            const std::vector<long> &vec_ids,
            const std::vector<FeatureItem> &vec_feature_item,
            std::vector<emb_any_type> &vec_result_emb) const noexcept;

        // 批量预测排序特征函数 - 使用缓存
        bool predict_rank_cache(
            const std::vector<RankItem> &vec_rank_item,
            std::vector<emb_any_type> &vec_result_emb) noexcept;

    private:
        std::string m_ModelName;
        std::mutex m_updateLock;
        std::atomic<int> m_dataIdx = 0;

        // 使用SharedPtr, 每次使用时复制一份, 不使用直接释放, 当引用为0时自动析构
        int m_EmbSize[2];
        TFModelMethodType m_RequestMethod[2];
        std::shared_ptr<TFTransData> m_spTFTransData[2];
        std::shared_ptr<TFModelBase> m_spTFModelBase[2];
        std::shared_ptr<TFModelGrpc> m_spTFModelGrpc[2];
        std::shared_ptr<std::vector<std::shared_ptr<EmbCacheData>>> m_spEmbCacheData[2];
    };

} // namespace TDPredict
