#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include "../Interface/ModelInterface.h"
#include "FMTrans.h"
#include "FMModelData.h"

namespace TDPredict
{
    class FMModel final : public ModelInterface
    {
    public:
        FMModel(const std::string &model_name) : ModelInterface(model_name) {}
        ~FMModel() {}

        // 预测函数
        virtual bool predict(RankItem &item) const noexcept override;

        // 批量预测函数
        virtual bool predict(std::vector<RankItem> &vec_item) const noexcept override;

    protected:
        void predict_feature(
            const TDPredict::FeatureItem &feature_item,
            const FMTransFormat &trans_format,
            const int data_idx,
            RankItem &item) const noexcept;

        void predict_score(RankItem &item, const int data_idx) const noexcept;

    public:
        // 初始化函数
        // [in] factor: 模型位数
        // [in] model_file_path: 模型文件地址
        // [in] filter_file_path: 转换格式地址
        bool Init(
            int factor,
            const std::string &model_file_path,
            const std::string &filter_file_path);

        // 更新模型文件
        // [in] factor: 模型位数
        // [in] model_file_path: 模型文件地址
        // [in] filter_file_path: 转换格式地址
        bool Update(
            int factor,
            const std::string &model_file_path,
            const std::string &filter_file_path);

    private:
        std::mutex m_updateModelLock; // 更新锁
        std::atomic<int> m_modelDataIdx = 0;
        FMModelData m_modelData[2];
        FMTransData m_transData[2];
    };
}
