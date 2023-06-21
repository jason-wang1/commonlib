#include "ModelConfig.h"
#include "glog/logging.h"
#include "Common/Function.h"

#include "../FMModel/FMModel.h"
#include "../TFModel/TFModel.h"
#include "../TFModel/SENet.h"
#include "../TFModel/SENetModel.h"

using namespace TDPredict::Config;

bool ModelConfig::Init(const std::string &json_file_path) noexcept
{
    return UpdateFile(json_file_path);
}

bool ModelConfig::UpdateFile(const std::string &json_file_path) noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    LOG(INFO) << "TDPredict::ModelConfig::UpdateFile"
              << ", json_file_path = " << json_file_path;
    if (json_file_path.empty())
    {
        m_lastErrorInfo = "TDPredict::ModelConfig::UpdateFile json_file_path empty";
        LOG(ERROR) << m_lastErrorInfo;
        return false;
    }

    // 读取配置
    if (ModelConfig::Error::OK != this->LoadJson(json_file_path))
    {
        LOG(ERROR) << "TDPredict::ModelConfig::UpdateFile LoadJson error"
                   << ", json_file_path = " << json_file_path;
        return false;
    }

    m_lastErrorInfo.clear();
    return true;
}

bool ModelConfig::UpdateJson(const std::string &json_data) noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    if (json_data.empty())
    {
        m_lastErrorInfo = "TDPredict::ModelConfig::UpdateJson() json_data empty.";
        LOG(ERROR) << m_lastErrorInfo;
        return false;
    }
    return ModelConfig::Error::OK == DecodeJsonData(json_data);
}

const std::string ModelConfig::GetConfigInfo() const noexcept
{
    // 先返回个空
    return "";
}

// 返回上次更新失败原因, 如果成功则返回空
const std::string ModelConfig::GetLastErrorInfo() const noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    return m_lastErrorInfo;
}

// 获取排序模型
const std::shared_ptr<TDPredict::ModelInterface> ModelConfig::GetModel(
    const std::string &platform,
    const std::string &model_name) const noexcept
{
    static const std::string Platform_FMModel = "FMModel";
    static const std::string Platform_TensorFlow = "TensorFlow";
    static const std::string Platform_SENetModel = "SENetModel";

    if (platform == Platform_FMModel)
    {
        auto &data = m_data[m_dataIdx];
        auto it = data.mapFMModel.find(model_name);
        if (it != data.mapFMModel.end())
        {
            return it->second;
        }
    }
    else if (platform == Platform_TensorFlow)
    {
        auto &data = m_data[m_dataIdx];
        auto it = data.mapTFModel.find(model_name);
        if (it != data.mapTFModel.end())
        {
            return it->second;
        }
    }
    else if (platform == Platform_SENetModel)
    {
        auto &data = m_data[m_dataIdx];
        auto it = data.mapSENetModel.find(model_name);
        if (it != data.mapSENetModel.end())
        {
            return it->second;
        }
    }

    return nullptr;
}

int32_t ModelConfig::LoadJson(const std::string &json_file_path)
{
    std::string json_data;
    Common::FastReadFile(json_file_path, json_data, false);
    if (json_data.empty())
    {
        m_lastErrorInfo = "LoadJson() read json_file empty";
        LOG(ERROR) << m_lastErrorInfo;
        return ModelConfig::Error::JsonDataEmpty;
    }
    return DecodeJsonData(json_data);
}

int32_t ModelConfig::DecodeJsonData(const std::string &jsonData)
{
    rapidjson::Document doc;
    if (doc.Parse(jsonData.data()).HasParseError())
    {
        m_lastErrorInfo = "DecodeJsonData() Parse jsonData HasParseError, err = " + doc.GetParseError();
        LOG(ERROR) << m_lastErrorInfo;
        return ModelConfig::Error::ParseJsonError;
    }

    // 缓存下标
    int data_idx = (m_dataIdx + 1) % 2;

    // 清理脏数据
    m_data[data_idx].clear();

    auto cfgErr = DecodeFMModelFolder(doc, data_idx);
    if (cfgErr != ModelConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeFMModelFolder Error.";
        return cfgErr;
    }

    cfgErr = DecodeTFSModelFolder(doc, data_idx);
    if (cfgErr != ModelConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeTFSModelFolder Error.";
        return cfgErr;
    }

    cfgErr = DecodeFMModel(doc, data_idx);
    if (cfgErr != ModelConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeFMModel Error.";
        return cfgErr;
    }

    cfgErr = DecodeTFModel(doc, data_idx);
    if (cfgErr != ModelConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeTFModel Error.";
        return cfgErr;
    }

    cfgErr = DecodeSENetModel(doc, data_idx);
    if (cfgErr != ModelConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeSENetModel Error.";
        return cfgErr;
    }

    // 配置读取完毕后, 初始化模型
    cfgErr = UpdateModel(data_idx);
    if (cfgErr != ModelConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() UpdateModel Error.";
        return cfgErr;
    }

    m_dataIdx = data_idx;
    return ModelConfig::Error::OK;
}

int32_t ModelConfig::DecodeFMModelFolder(
    const rapidjson::Document &doc,
    const int data_idx)
{
    if (!doc.HasMember("FMModelFolder"))
    {
        return ModelConfig::Error::OK;
    }

    if (!doc["FMModelFolder"].IsArray())
    {
        LOG(ERROR) << "DecodeFMModelFolder() Parse jsonData FMModelFolder Not Array.";
        return ModelConfig::Error::DecodeFMModelFolderError;
    }

    auto array = doc["FMModelFolder"].GetArray();
    auto array_size = array.Size();
    auto &vecFolder = m_data[data_idx].vecFMModelFolder;
    vecFolder.clear();
    vecFolder.reserve(array_size);
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        if (!array[idx].IsString())
        {
            LOG(ERROR) << "DecodeFMModelFolder() Parse jsonData, FMModelFolder idx = " << idx << " Not String.";
            return ModelConfig::Error::DecodeFMModelFolderError;
        }
        vecFolder.emplace_back(std::string(array[idx].GetString(), array[idx].GetStringLength()));
    }
    return ModelConfig::Error::OK;
}

int32_t ModelConfig::DecodeTFSModelFolder(
    const rapidjson::Document &doc,
    const int data_idx)
{
    if (!doc.HasMember("TFSModelFolder"))
    {
        return ModelConfig::Error::OK;
    }

    if (!doc["TFSModelFolder"].IsArray())
    {
        LOG(ERROR) << "DecodeTFSModelFolder() Parse jsonData TFSModelFolder Not Array.";
        return ModelConfig::Error::DecodeTFSModelFolderError;
    }

    auto array = doc["TFSModelFolder"].GetArray();
    auto array_size = array.Size();
    auto &vecFolder = m_data[data_idx].vecTFSModelFolder;
    vecFolder.clear();
    vecFolder.reserve(array_size);
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        if (!array[idx].IsString())
        {
            LOG(ERROR) << "DecodeTFSModelFolder() Parse jsonData, TFSModelFolder idx = " << idx << " Not String.";
            return ModelConfig::Error::DecodeTFSModelFolderError;
        }
        vecFolder.emplace_back(std::string(array[idx].GetString(), array[idx].GetStringLength()));
    }
    return ModelConfig::Error::OK;
}

int32_t ModelConfig::DecodeFMModel(
    const rapidjson::Document &doc,
    const int data_idx)
{
    if (!doc.HasMember("FMModel"))
    {
        return ModelConfig::Error::OK;
    }

    if (!doc["FMModel"].IsArray())
    {
        LOG(ERROR) << "DecodeFMModel() Parse jsonData FMModel Not Array.";
        return ModelConfig::Error::DecodeFMModelError;
    }

    auto array = doc["FMModel"].GetArray();
    auto array_size = array.Size();
    m_data[data_idx].vecFMModelData.clear();
    m_data[data_idx].vecFMModelData.resize(array_size);
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        if (!array[idx].IsObject())
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Object"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }

        auto item = array[idx].GetObject();
        auto &model_data = m_data[data_idx].vecFMModelData[idx];

        if (item.HasMember("ServiceName") && item["ServiceName"].IsString())
        {
            model_data.ServiceName = std::string(item["ServiceName"].GetString(),
                                                 item["ServiceName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Find ServiceName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }

        if (item.HasMember("ModelName") && item["ModelName"].IsString())
        {
            model_data.ModelName = std::string(item["ModelName"].GetString(),
                                               item["ModelName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Find ModelName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }

        if (item.HasMember("Factor") && item["Factor"].IsInt())
        {
            model_data.Factor = item["Factor"].GetInt();
        }
        else
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Find Factor"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }

        // 模型版本号
        if (item.HasMember("Version") && item["Version"].IsString())
        {
            model_data.Version = std::string(item["Version"].GetString(),
                                             item["Version"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Find Version"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }

        if (item.HasMember("ModelFileName") && item["ModelFileName"].IsString())
        {
            model_data.ModelFileName = std::string(item["ModelFileName"].GetString(),
                                                   item["ModelFileName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Find ModelFileName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }

        if (item.HasMember("FilterFileName") && item["FilterFileName"].IsString())
        {
            model_data.FilterFileName = std::string(item["FilterFileName"].GetString(),
                                                    item["FilterFileName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeFMModel() Parse jsonData"
                       << ", FMModel Not Find FilterFileName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeFMModelError;
        }
    }
    return ModelConfig::Error::OK;
}

int32_t ModelConfig::DecodeTFModel(
    const rapidjson::Document &doc,
    const int data_idx)
{
    if (!doc.HasMember("TFModel"))
    {
        return ModelConfig::Error::OK;
    }

    if (!doc["TFModel"].IsArray())
    {
        LOG(ERROR) << "DecodeTFModel() Parse jsonData TFModel Not Array.";
        return ModelConfig::Error::DecodeFMModelError;
    }

    auto &data = m_data[data_idx];
    auto array = doc["TFModel"].GetArray();
    auto array_size = array.Size();
    data.vecTFModelData.clear();
    data.vecTFModelData.resize(array_size);
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        if (!array[idx].IsObject())
        {
            LOG(ERROR) << "DecodeTFModel() Parse jsonData"
                       << ", TFModel Not Object"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeTFModelError;
        }

        auto item = array[idx].GetObject();
        auto &model_data = data.vecTFModelData[idx];

        // 服务名称
        if (item.HasMember("ServiceName") && item["ServiceName"].IsString())
        {
            model_data.ServiceName = std::string(item["ServiceName"].GetString(),
                                                 item["ServiceName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeTFModel() Parse jsonData"
                       << ", TFModel Not Find ServiceName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeTFModelError;
        }

        // 模型名称
        if (item.HasMember("ModelName") && item["ModelName"].IsString())
        {
            model_data.ModelName = std::string(item["ModelName"].GetString(),
                                               item["ModelName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeTFModel() Parse jsonData"
                       << ", TFModel Not Find ModelName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeTFModelError;
        }

        // 模型版本号
        if (item.HasMember("Version") && item["Version"].IsString())
        {
            model_data.Version = std::string(item["Version"].GetString(),
                                             item["Version"].GetStringLength());
        }

        // 远程调用地址
        if (item.HasMember("TFSAddrList") && item["TFSAddrList"].IsArray())
        {
            auto tfs_addr_array = item["TFSAddrList"].GetArray();
            auto tfs_addr_array_size = tfs_addr_array.Size();
            auto &vecTFSAddr = model_data.TFSAddrList;
            vecTFSAddr.resize(tfs_addr_array_size);
            for (rapidjson::SizeType tfs_addr_idx = 0; tfs_addr_idx < tfs_addr_array_size; tfs_addr_idx++)
            {
                if (!tfs_addr_array[tfs_addr_idx].IsString())
                {
                    LOG(ERROR) << "DecodeTFModel() Parse jsonData"
                               << ", TFSAddrList tfs_addr_idx = " << tfs_addr_idx << " Not String.";
                    return ModelConfig::Error::DecodeTFModelError;
                }

                vecTFSAddr[tfs_addr_idx] = std::string(tfs_addr_array[tfs_addr_idx].GetString(),
                                                       tfs_addr_array[tfs_addr_idx].GetStringLength());
            }
        }

        // 模型远程调用请求超时时长
        if (item.HasMember("Timeout") && item["Timeout"].IsInt())
        {
            model_data.Timeout = item["Timeout"].GetInt();
        }

        // 转换文件名称
        if (item.HasMember("TransFileName") && item["TransFileName"].IsString())
        {
            model_data.TransFileName = std::string(item["TransFileName"].GetString(),
                                                   item["TransFileName"].GetStringLength());
        }

        // 模型签名
        if (item.HasMember("SignatureName") && item["SignatureName"].IsString())
        {
            model_data.SignatureName = std::string(item["SignatureName"].GetString(),
                                                   item["SignatureName"].GetStringLength());
        }

        // 是否使用DropoutKeep字段
        if (item.HasMember("UseDropoutKeep") && item["UseDropoutKeep"].IsBool())
        {
            model_data.UseDropoutKeep = item["UseDropoutKeep"].GetBool();
        }

        // 是否使用UseUserType字段
        if (item.HasMember("UseUserType") && item["UseUserType"].IsBool())
        {
            model_data.UseUserType = item["UseUserType"].GetBool();
        }

        // RequestMethod
        if (item.HasMember("RequestMethod") && item["RequestMethod"].IsString())
        {
            const std::string method = std::string(item["RequestMethod"].GetString(),
                                                   item["RequestMethod"].GetStringLength());
            if ("Restful" == method)
            {
                model_data.RequestMethod = TFModelMethodType::Restful;
            }
            else if ("GRPC" == method)
            {
                model_data.RequestMethod = TFModelMethodType::GRPC;
            }
            else
            {
                LOG(ERROR) << "DecodeTFModel() RequestMethod Not equal"
                           << ", method = " << method;
                return ModelConfig::Error::DecodeTFModelError;
            }
        }

        // 模型签名
        if (item.HasMember("OutputName") && item["OutputName"].IsString())
        {
            model_data.OutputName = std::string(item["OutputName"].GetString(),
                                                item["OutputName"].GetStringLength());
        }

        // 如果配置TFS地址为空, 则采用默认地址
        if (model_data.TFSAddrList.empty())
        {
            if (TFModelMethodType::Restful == model_data.RequestMethod)
            {
                model_data.TFSAddrList = m_vecDefaultRestfulAddrList;
            }
            else if (TFModelMethodType::GRPC == model_data.RequestMethod)
            {
                model_data.TFSAddrList = m_vecDefaultGRPCAddrList;
            }
            else
            {
                LOG(ERROR) << "DecodeTFModel() model_data.RequestMethod Not equal"
                           << ", model_data.RequestMethod = " << model_data.RequestMethod;
                return ModelConfig::Error::DecodeTFModelError;
            }
        }
    }
    return ModelConfig::Error::OK;
}

int32_t ModelConfig::DecodeSENetModel(const rapidjson::Document &doc, const int data_idx)
{
    if (!doc.HasMember("SENetModel"))
    {
        return ModelConfig::Error::OK;
    }

    if (!doc["SENetModel"].IsArray())
    {
        LOG(ERROR) << "DecodeSENetModel() Parse jsonData SENetModel Not Array.";
        return ModelConfig::Error::DecodeSENetModelError;
    }

    auto &data = m_data[data_idx];
    auto array = doc["SENetModel"].GetArray();
    auto array_size = array.Size();
    data.vecSENetModelData.clear();
    data.vecSENetModelData.resize(array_size);
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        if (!array[idx].IsObject())
        {
            LOG(ERROR) << "DecodeSENetModel() Parse jsonData"
                       << ", SENetModel Not Object"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeSENetModelError;
        }

        auto item = array[idx].GetObject();
        auto &model_data = data.vecSENetModelData[idx];

        // 模型名称
        if (item.HasMember("ModelName") && item["ModelName"].IsString())
        {
            model_data.ModelName = std::string(item["ModelName"].GetString(),
                                               item["ModelName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeSENetModel() Parse jsonData"
                       << ", SENetModel Not Find ModelName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeSENetModelError;
        }

        // 用户塔模型名称
        if (item.HasMember("UserModelName") && item["UserModelName"].IsString())
        {
            model_data.UserModelName = std::string(item["UserModelName"].GetString(),
                                                   item["UserModelName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeSENetModel() Parse jsonData"
                       << ", SENetModel Not Find UserModelName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeSENetModelError;
        }

        // 物料塔模型名称
        if (item.HasMember("ItemModelName") && item["ItemModelName"].IsString())
        {
            model_data.ItemModelName = std::string(item["ItemModelName"].GetString(),
                                                   item["ItemModelName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeSENetModel() Parse jsonData"
                       << ", SENetModel Not Find ItemModelName"
                       << ", idx = " << idx;
            return ModelConfig::Error::DecodeSENetModelError;
        }
    }
    return ModelConfig::Error::OK;
}

int32_t ModelConfig::UpdateModel(const int dataIdx)
{
    static const std::string latest = "latest"; // 最新版本号

    auto &data = m_data[dataIdx];

    // 更新FM模型
    for (const auto &model : data.vecFMModelData)
    {
        // 先查找模型文件
        std::filesystem::path model_dir;
        std::filesystem::path model_filePath;
        std::filesystem::path filter_filePath;
        for (const auto &folder : data.vecFMModelFolder)
        {
            model_dir.append(folder);
            model_dir.append(model.ServiceName);
            model_dir.append(model.ModelName);

            // 获取版本号
            if (model.Version == latest)
            {
                std::string version;
                if (!TDPredict::GetLatestVersion(model_dir, version))
                {
                    LOG(ERROR) << "ModelConfig::UpdateModel() GetLatestVersion Failed"
                               << ", FMModel.ServiceName = " << model.ServiceName
                               << ", FMModel.ModelName = " << model.ModelName
                               << ", model_dir = " << model_dir;
                    continue;
                }
                model_dir.append(version);
            }
            else
            {
                model_dir.append(model.Version);
            }

            if (!std::filesystem::is_directory(model_dir))
            {
                LOG(ERROR) << "ModelConfig::UpdateModel() filePath Not directory"
                           << ", model_dir = " << model_dir;
                model_dir.clear();
                continue;
            }

            model_filePath = model_dir;
            model_filePath.append(model.ModelFileName);

            filter_filePath = model_dir;
            filter_filePath.append(model.FilterFileName);
            if (std::filesystem::is_regular_file(model_filePath) &&
                std::filesystem::is_regular_file(filter_filePath))
            {
                break; // 找到文件了, break出去
            }
            model_filePath.clear();
            filter_filePath.clear();
        }
        if (model_filePath.empty() || filter_filePath.empty())
        {
            LOG(ERROR) << "ModelConfig::UpdateModel() FMModel File Not Found"
                       << ", FMModel.ServiceName = " << model.ServiceName
                       << ", FMModel.ModelName = " << model.ModelName
                       << ", FMModel.ModelFileName = " << model.ModelFileName
                       << ", FMModel.FilterFileName = " << model.FilterFileName;
            return ModelConfig::Error::UpdateModelError;
        }

        // 找到 FMModel
        std::shared_ptr<FMModel> spModel = nullptr;
        auto it = data.mapFMModel.find(model.ModelName);
        if (it != data.mapFMModel.end())
        {
            spModel = it->second;
        }

        if (spModel == nullptr)
        {
            spModel = std::make_shared<FMModel>(model.ModelName);
            data.mapFMModel[model.ModelName] = spModel;
        }

        // 更新FMModel
        if (!spModel->Update(model.Factor, model_filePath, filter_filePath))
        {
            LOG(ERROR) << "ModelConfig::UpdateModel() FMModel Update Failed"
                       << ", FMModel.ServiceName = " << model.ServiceName
                       << ", FMModel.ModelName = " << model.ModelName
                       << ", model_filePath = " << model_filePath
                       << ", filter_filePath = " << filter_filePath;
            return ModelConfig::Error::UpdateModelError;
        }
        else
        {
            LOG(INFO) << "ModelConfig::UpdateModel() FMModule Update Succ"
                      << ", FMModel.ServiceName = " << model.ServiceName
                      << ", FMModel.ModelName = " << model.ModelName
                      << ", model_filePath = " << model_filePath
                      << ", filter_filePath = " << filter_filePath;

            // 记录模型名称对应文件地址
            data.mapFMModel2ModelFilePath[model.ModelName] = model_filePath;
            data.mapFMModel2FilterFilePath[model.FilterFileName] = filter_filePath;
        }
    }

    // 更新TFModel
    for (auto &model : data.vecTFModelData)
    {
        // 先查找 feature_trans 文件
        std::filesystem::path trans_filePath;
        for (const auto &folder : data.vecTFSModelFolder)
        {
            trans_filePath.append(folder);
            trans_filePath.append(model.ServiceName);
            trans_filePath.append(model.ModelName);

            // 获取版本号
            if (model.Version == latest)
            {
                std::string version;
                if (!TDPredict::GetLatestVersion(trans_filePath, version))
                {
                    LOG(ERROR) << "ModelConfig::UpdateModel() GetLatestVersion Failed"
                               << ", model.ServiceName = " << model.ServiceName
                               << ", model.ModelName = " << model.ModelName
                               << ", trans_filePath = " << trans_filePath;
                    continue;
                }
                model.Version = version;
                trans_filePath.append(version);
            }
            else
            {
                trans_filePath.append(model.Version);
            }

            if (!std::filesystem::is_directory(trans_filePath))
            {
                LOG(ERROR) << "ModelConfig::UpdateModel() trans_filePath Not directory"
                           << ", trans_filePath = " << trans_filePath;
                trans_filePath.clear();
                continue;
            }

            trans_filePath.append(model.TransFileName);
            if (std::filesystem::is_regular_file(trans_filePath))
            {
                break; // 找到文件了, break出去
            }
            trans_filePath.clear();
        }
        if (trans_filePath.empty())
        {
            LOG(ERROR) << "ModelConfig::UpdateModel() TFModel File Not Found"
                       << ", model.ServiceName = " << model.ServiceName
                       << ", model.ModelName = " << model.ModelName
                       << ", model.Version = " << model.Version
                       << ", model.TransFileName = " << model.TransFileName;
            return ModelConfig::Error::UpdateModelError;
        }

        // 找到 TFModel
        std::shared_ptr<TFModel> spModel = nullptr;
        auto it = data.mapTFModel.find(model.ModelName);
        if (it != data.mapTFModel.end())
        {
            spModel = it->second;
        }
        if (spModel == nullptr)
        {
            spModel = std::make_shared<TFModel>(model.ModelName);
            data.mapTFModel[model.ModelName] = spModel;
        }

        // 更新TFModel
        const int version = (model.Version == latest) ? 0 : atoi(model.Version.c_str());
        if (!spModel->Update(model.TFSAddrList, model.SignatureName,
                             model.Timeout, trans_filePath,
                             model.UseDropoutKeep, version,
                             model.RequestMethod, model.OutputName,
                             model.UseUserType))
        {
            LOG(ERROR) << "ModelConfig::UpdateModel() TFModel Update Failed"
                       << ", model.ServiceName = " << model.ServiceName
                       << ", model.ModelName = " << model.ModelName
                       << ", model.Version = " << model.Version
                       << ", trans_filePath = " << trans_filePath;
            return ModelConfig::Error::UpdateModelError;
        }
        else
        {
            LOG(INFO) << "ModelConfig::UpdateModel() TFModel Update Succ"
                      << ", model.ServiceName = " << model.ServiceName
                      << ", model.ModelName = " << model.ModelName
                      << ", model.Version = " << model.Version
                      << ", trans_filePath = " << trans_filePath;

            // 记录模型名称对应文件地址
            data.mapTFModel2TransFilePath[model.ModelName] = trans_filePath;
        }
    }

    // 更新SENetModel
    for (const auto &model : data.vecSENetModelData)
    {
        // 找到 SENetModel
        std::shared_ptr<SENetModel> spModel = nullptr;
        {
            auto it = data.mapSENetModel.find(model.ModelName);
            if (it != data.mapSENetModel.end())
            {
                spModel = it->second;
            }
            if (spModel == nullptr)
            {
                spModel = std::make_shared<SENetModel>(model.ModelName);
                data.mapSENetModel[model.ModelName] = spModel;
            }
        }

        // 更新 SENetModel
        if (!spModel->Update(model.UserModelName, model.ItemModelName))
        {
            LOG(ERROR) << "ModelConfig::UpdateModel() SENetModel Update Failed"
                       << ", model.ModelName = " << model.ModelName
                       << ", model.UserModelName = " << model.UserModelName
                       << ", model.ItemModelName = " << model.ItemModelName;
            return ModelConfig::Error::UpdateModelError;
        }
        else
        {
            LOG(INFO) << "ModelConfig::UpdateModel() SENetModel Update Succ"
                      << ", model.ModelName = " << model.ModelName
                      << ", model.UserModelName = " << model.UserModelName
                      << ", model.ItemModelName = " << model.ItemModelName;
        }
    }

    return ModelConfig::Error::OK;
}
