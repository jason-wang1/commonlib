#pragma once
#include <vector>
#include <string>

namespace TDPredict
{
    struct TFSInputData;
    class TFModelBase final
    {
    public:
        TFModelBase(
            const std::vector<std::string> &vec_tfs_addrs,
            const std::string &signature_name,
            const std::string &model_name,
            const int timeout,
            const bool use_dropout_keep,
            const int version);

        virtual ~TFModelBase(){};

        // 批量预测函数
        bool predict_base(
            const std::vector<TFSInputData> &vec_tfs_inputs,
            std::vector<std::string> &vec_tfs_results) const noexcept;

        std::string make_tfs_post_msg(const TFSInputData &tfs_input);

    private:
        static bool GetRandAddr(
            const std::vector<std::string> &vec_tfs_addrs,
            std::string &tfs_addr);

        static std::string make_tfs_batch_post_msg(
            const std::string &signatureName,
            const std::vector<TFSInputData> &vec_tfs_inputs,
            const int begin, const int end,
            const bool use_dropout_keep);

        static bool get_tfs_batch_post_result(
            const std::string &json_result,
            const int begin,
            const int end,
            std::vector<std::string> &vec_tfs_results);

        static std::string get_format_json(const std::string &json);

        static std::string make_tfs_request_URL(
            const std::string &tfs_addr,
            const std::string &model_name)
        {
            return "http://" + tfs_addr + "/v1/models/" + model_name + ":predict";
        }

    private:
        int m_Timeout;
        std::string m_ModelName;
        int m_Version;
        std::string m_SignatureName;
        std::vector<std::string> m_TFSAddrList;
        bool m_UseDropoutKeep;
    };

} // namespace TDPredict
