#pragma once
#include <memory>
#include <atomic>
#include <mutex>

#include "TFTrans.h"
#include "TFModelBase.h"
#include "TFModelGrpc.h"
#include "../Interface/ModelInterface.h"

namespace TDPredict
{
    class TFModel final : public ModelInterface
    {
    public:
        TFModel(const std::string &model_name)
            : ModelInterface(model_name) {}
        virtual ~TFModel() {}

        // 更新配置
        bool Update(
            const std::vector<std::string> &vec_tfs_addrs,
            const std::string &signature_name,
            const int timeout,
            const std::string &trans_file_path,
            const bool use_dropout_keep,
            const int version,
            const TFModelMethodType request_method,
            const std::string &output_name,
            const bool use_user_type) noexcept;

        // 预测函数
        virtual bool predict(RankItem &item) const noexcept override;

        // 批量预测函数
        virtual bool predict(std::vector<RankItem> &vec_rank_item) const noexcept override;

    private:
        bool GenerateTFSInput(
            const std::vector<RankItem> &vec_rank_item,
            const TFTransData &trans_data,
            std::vector<TDPredict::TFSInputData> &vec_tfs_inputs) const noexcept;

    private:
        std::mutex m_updateLock; // 更新锁
        std::atomic<int> m_dataIdx = 0;
        TFModelMethodType m_RequestMethod[2];
        std::shared_ptr<TFTransData> m_spTFTransData[2];
        std::shared_ptr<TFModelBase> m_spTFModelBase[2];
        std::shared_ptr<TFModelGrpc> m_spTFModelGrpc[2];
    };

} // namespace TDPredict
