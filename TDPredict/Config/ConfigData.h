#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "../Interface/ModelInterface.h"

namespace TDPredict
{
    class SENet;
    class FMModel;
    class TFModel;
    class SENetModel;

    // FM 模型配置
    struct FMModelConfigData
    {
        std::string ServiceName;                       // 服务名称
        std::string ModelName;                         // 模型名称
        int Factor;                                    // 模型位数
        std::string Version = "latest";                // 模型版本号, latest标识自动获取最新版本
        std::string ModelFileName = "fm_model";        // 模型文件名
        std::string FilterFileName = "feature_filter"; // 特征筛选文件名 - feature_filter
    };

    // TensorFlow 模型配置
    struct TFModelConfigData
    {
        std::string ServiceName;                       // 服务名称
        std::string ModelName;                         // 模型名称
        std::vector<std::string> TFSAddrList;          // 模型调用地址
        int Timeout = 300;                             // 模型调用超时时长
        std::string Version = "latest";                // 模型版本号, latest标识自动获取最新版本
        std::string TransFileName = "feature_trans";   // 转换文件名 - feature_trans
        std::string SignatureName = "serving_default"; // 签名 - serving_default
        bool UseDropoutKeep = false;                   // 是否使用DropoutKeep字段
        TFModelMethodType RequestMethod = Restful;     // 模型请求调用方案
        std::string OutputName = "predictions";        // 模型读取返回值名称
        bool UseUserType = false;                      // 是否使用UserType字段
    };

    struct SENetItemConfigData
    {
        std::string ServiceName;                       // 服务名称
        std::string ModelName;                         // 模型名称
        std::vector<std::string> TFSAddrList;          // TensorFlow Serving 调用地址
        int Timeout = 300;                             // TensorFlow Serving 调用超时时长
        std::string Version = "latest";                // 模型版本号, latest标识自动获取最新版本
        std::string TransFileName = "feature_trans";   // 转换文件名 - feature_trans
        std::string SignatureName = "serving_default"; // 签名 - serving_default
        bool UseDropoutKeep = false;                   // 是否使用DropoutKeep字段
        TFModelMethodType RequestMethod = Restful;     // 模型请求调用方案
        std::string OutputName = "embedding";          // 模型读取返回值名称
        int EmbSize = 128;                             // 模型返回Emb长度, 默认128
    };

    // SENet 模型配置, 调用TF模型
    struct SENetModelConfigData
    {
        std::string ModelName;     // 模型名称
        std::string UserModelName; // TensorFlow 用户模型名称
        std::string ItemModelName; // TensorFlow 物料模型名称
    };

    struct ModelConfigData
    {
    public:
        std::vector<std::string> vecFMModelFolder;  // 查找FM模型文件夹地址
        std::vector<std::string> vecTFSModelFolder; // 查找TFS模型文件夹地址

        // 模型配置
        std::vector<FMModelConfigData> vecFMModelData;       // FMModel 配置
        std::vector<TFModelConfigData> vecTFModelData;       // TFModel 配置
        std::vector<SENetModelConfigData> vecSENetModelData; // SENetModel 配置

    public:
        // 模型
        std::unordered_map<std::string, std::shared_ptr<FMModel>> mapFMModel;
        std::unordered_map<std::string, std::shared_ptr<TFModel>> mapTFModel;
        std::unordered_map<std::string, std::shared_ptr<SENetModel>> mapSENetModel;

        // 模型对应文件路径
        std::unordered_map<std::string, std::string> mapFMModel2ModelFilePath;
        std::unordered_map<std::string, std::string> mapFMModel2FilterFilePath;
        std::unordered_map<std::string, std::string> mapTFModel2TransFilePath;

    public:
        void clear()
        {
            vecFMModelFolder.clear();
            vecTFSModelFolder.clear();

            vecFMModelData.clear();
            vecTFModelData.clear();
            vecSENetModelData.clear();

            mapFMModel.clear();
            mapTFModel.clear();
            mapSENetModel.clear();

            mapFMModel2ModelFilePath.clear();
            mapFMModel2FilterFilePath.clear();
            mapTFModel2TransFilePath.clear();
        }
    };

    struct SENetConfigData
    {
    public:
        std::vector<std::string> vecTFSModelFolder;       // 查找TFS模型文件夹地址
        std::vector<std::string> vecDefaultSENetAddrList; // SENet 默认调用地址
        std::vector<SENetItemConfigData> vecSENetData;    // SENet 配置, 使用TF模型
        std::unordered_map<std::string, std::shared_ptr<SENet>> mapSENet;
        std::unordered_map<std::string, std::string> mapSENet2TransFilePath;

    public:
        void clear()
        {
            vecTFSModelFolder.clear();
            vecDefaultSENetAddrList.clear();
            vecSENetData.clear();
            mapSENet.clear();
            mapSENet2TransFilePath.clear();
        }
    };

    // 策略
    struct StrategyConfigData
    {
        std::string Platform;       // 模型平台
        std::string FeatureName;    // 特征名称
        std::string ModelName;      // 模型名称
        int TruncCount;             // 截断数量
        int CalcCount;              // 计算数量
        std::string BeforeStrategy; // 前置策略 - 默认空策略 - 模型计算前, 加特征 Or 判断条件是否执行这个模型...
        std::string ScoreStrategy;  // 分数策略 - 默认空策略 - 模型计算后, 可针对Item修改分数, 影响排序顺序
        std::string EndStrategy;    // 结束策略 - 默认空策略 - 模型排序后, 对有序Item分数修改, 不改变顺序
    };

    struct RankStrategyConfigData
    {
        std::vector<StrategyConfigData> StrategyList;
        std::string BeforeStrategy; // 前置策略 - 默认空策略 - 在排序计算前执行操作, 判断条件是否执行排序层
        std::string ScoreStrategy;  // 分数策略 - 单模型空策略 - 多模型默认加和 - 可用加权比例决定不同模型分数占比 Or 直接修改分数
        std::string EndStrategy;    // 结束策略 - 默认空策略 - 排序层分数计算结束后, 对有序的模型层分数进行调整, 不改变顺序
    };

    using MapExp2RankStrategy = std::unordered_map<int, RankStrategyConfigData>;

    struct RankExpConfigData
    {
    public:
        MapExp2RankStrategy vecRankStrategy[TDPredict::RankType::Count]; // Idx = RankType

    public:
        void clear()
        {
            for (int idx = TDPredict::RankType::Begin; idx < TDPredict::RankType::End; idx++)
            {
                vecRankStrategy[idx].clear();
            }
        }
    };

    inline bool GetLatestVersion(
        const std::string &path,
        std::string &result)
    {
        auto store_model_path = std::filesystem::path(path);
        bool find = false;
        long latest_version = 0;

        // 如果不存在, 返回错误
        if (!std::filesystem::exists(store_model_path))
        {
            // 如果想新加一类模型, 需要手动建立文件夹
            result = "store_model_path " + store_model_path.string() + " not exists";
            return false;
        }

        // 如果存在, 但是不是文件夹, 那只能返回错误
        if (!std::filesystem::is_directory(store_model_path))
        {
            // tf模型的路径必须是文件夹
            result = "store_model_path" + store_model_path.string() + " must be a folder";
            return false;
        }

        // 遍历文件夹, 查询最大的版本号
        for (auto &tf_model_iter : std::filesystem::directory_iterator(store_model_path))
        {
            std::string filename = tf_model_iter.path().filename();
            long version = atol(filename.c_str());
            if (!find || version > latest_version)
            {
                // 版本号大于等于新的版本号
                latest_version = version;
                find = true;
            }
        }

        // 如果没有找到
        if (!find)
        {
            result = "store_model_path" + store_model_path.string() + " is empty";
            return false;
        }

        // 返回结果
        result = std::to_string(latest_version);
        return true;
    }

} // namespace TDPredict
