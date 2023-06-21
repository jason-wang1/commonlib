#pragma once
#include <memory>
#include <atomic>
#include <mutex>

#include "TFTrans.h"
#include "TFModelBase.h"
#include "../Interface/ModelInterface.h"

namespace TDPredict
{
    class SENet;
    class SENetModel final : public ModelInterface
    {
    public:
        SENetModel(const std::string &model_name)
            : ModelInterface(model_name) {}
        virtual ~SENetModel() {}

        // 更新配置
        bool Update(
            const std::string &user_model_name,
            const std::string &item_model_name) noexcept;

    public:
        // 预测函数
        virtual bool predict(RankItem &item) const noexcept override;

        // 批量预测函数
        virtual bool predict(std::vector<RankItem> &vec_rank_item) const noexcept override;

    private:
        std::mutex m_updateLock;
        std::atomic<int> m_dataIdx = 0;
        std::shared_ptr<SENet> m_spUserSENet[2]; // 用户SENet模型
        std::shared_ptr<SENet> m_spItemSENet[2]; // 物料SENet模型
    };

} // namespace TDPredict
