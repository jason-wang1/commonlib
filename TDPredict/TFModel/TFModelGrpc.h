#pragma once
#include <memory>
#include <vector>
#include <any>
#include <future>

#include "Interface/ModelInterface.h"
#include "TFServingClient.hpp"

namespace TDPredict
{
    struct TFSInputData
    {
        std::vector<int> index;
        std::vector<score_type> value;
        TFModelUserType user_type;

        void clear()
        {
            index.clear();
            value.clear();
            user_type = TFModelUserType::Default;
        }
    };

    class TFModelGrpc final
    {
    public:
        struct ScoreData
        {
            const TFModelGrpc *obj;
            std::vector<TDPredict::score_type> *vec_score;
            int begin;
            int end;
        };
        using ScoreDataPtr = std::shared_ptr<ScoreData>;

        struct EmbAnyData
        {
            const TFModelGrpc *obj;
            std::vector<TDPredict::emb_any_type> *vec_emb_any;
            int begin;
            int end;
        };
        using EmbAnyDataPtr = std::shared_ptr<EmbAnyData>;

    public:
        TFModelGrpc(
            const std::vector<std::string> &vec_tfs_addrs,
            const std::string &signature_name,
            const std::string &model_name,
            const int timeout,
            const bool use_dropout_keep,
            const int version,
            const std::string &output_name,
            const bool use_user_type);

        virtual ~TFModelGrpc(){};

        // 批量预测分数函数
        bool predict_score(
            const std::vector<TFSInputData> &vec_tfs_inputs,
            std::vector<TDPredict::score_type> &vec_tfs_results) const noexcept;

        // 批量预测Emb函数
        bool predict_emb(
            const std::vector<TFSInputData> &vec_tfs_inputs,
            std::vector<TDPredict::emb_any_type> &vec_emb_any) const noexcept;

    private:
        bool GenerateRequestProto(
            const std::vector<TFSInputData> &vec_tfs_input,
            const int begin, const int end,
            RequestProto &request) const noexcept;

        static bool CallBackScoreFunc(
            const grpc::Status status,
            ResponseProto &response,
            std::any data);

        static bool CallBackEmbFunc(
            const grpc::Status status,
            ResponseProto &response,
            std::any data);

    private:
        static constexpr int per_batch_size = 256; // 分批调用, 每一批次数量
        const TFservingClient client;
        std::vector<std::string> m_TFSAddrList;
        std::string m_ModelName;
        std::string m_SignatureName;
        int m_Timeout;
        int m_Version;
        bool m_UseDropoutKeep;
        std::string m_OutputName;
        bool m_UseUserType;
    };

} // namespace TDPredict
