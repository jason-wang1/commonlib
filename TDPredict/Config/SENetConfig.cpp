#include "SENetConfig.h"
#include "glog/logging.h"
#include "Common/Function.h"

#include "../TFModel/SENet.h"

using namespace TDPredict::Config;

bool SENetConfig::Init(const std::string &json_file_path) noexcept
{
    return UpdateFile(json_file_path);
}

bool SENetConfig::UpdateFile(const std::string &json_file_path) noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    LOG(INFO) << "TDPredict::SENetConfig::UpdateFile"
              << ", json_file_path = " << json_file_path;
    if (json_file_path.empty())
    {
        m_lastErrorInfo = "TDPredict::SENetConfig::UpdateFile json_file_path empty";
        LOG(ERROR) << m_lastErrorInfo;
        return false;
    }

    //读取配置
    if (SENetConfig::Error::OK != this->LoadJson(json_file_path))
    {
        LOG(ERROR) << "TDPredict::SENetConfig::UpdateFile LoadJson error"
                   << ", json_file_path = " << json_file_path;
        return false;
    }

    m_lastErrorInfo.clear();
    return true;
}

bool SENetConfig::UpdateJson(const std::string &json_data) noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    if (json_data.empty())
    {
        m_lastErrorInfo = "TDPredict::SENetConfig::UpdateJson() json_data empty.";
        LOG(ERROR) << m_lastErrorInfo;
        return false;
    }
    return SENetConfig::Error::OK == DecodeJsonData(json_data);
}

const std::string SENetConfig::GetConfigInfo() const noexcept
{
    // 先返回个空
    return "";
}

// 返回上次更新失败原因, 如果成功则返回空
const std::string SENetConfig::GetLastErrorInfo() const noexcept
{
    std::lock_guard<std::mutex> lg(m_update_lock);
    return m_lastErrorInfo;
}

// 获取排序模型
const std::shared_ptr<TDPredict::SENet> SENetConfig::GetSENet(
    const std::string &model_name) const noexcept
{
    auto &data = m_data[m_dataIdx];
    auto it = data.mapSENet.find(model_name);
    if (it != data.mapSENet.end())
    {
        return it->second;
    }
    return nullptr;
}

int32_t SENetConfig::LoadJson(const std::string &json_file_path)
{
    std::string json_data;
    Common::FastReadFile(json_file_path, json_data, false);
    if (json_data.empty())
    {
        m_lastErrorInfo = "LoadJson() read json_file empty";
        LOG(ERROR) << m_lastErrorInfo;
        return SENetConfig::Error::JsonDataEmpty;
    }
    return DecodeJsonData(json_data);
}

int32_t SENetConfig::DecodeJsonData(const std::string &jsonData)
{
    rapidjson::Document doc;
    if (doc.Parse(jsonData.data()).HasParseError())
    {
        m_lastErrorInfo = "DecodeJsonData() Parse jsonData HasParseError, err = " + doc.GetParseError();
        LOG(ERROR) << m_lastErrorInfo;
        return SENetConfig::Error::ParseJsonError;
    }

    // 缓存下标
    int data_idx = (m_dataIdx + 1) % 2;

    // 清理脏数据
    m_data[data_idx].clear();

    auto cfgErr = DecodeTFSModelFolder(doc, data_idx);
    if (cfgErr != SENetConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeTFSModelFolder Error.";
        return cfgErr;
    }

    cfgErr = DecodeSENet(doc, data_idx);
    if (cfgErr != SENetConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() DecodeSENet Error.";
        return cfgErr;
    }

    // 配置读取完毕后, 初始化模型
    cfgErr = UpdateModel(data_idx);
    if (cfgErr != SENetConfig::Error::OK)
    {
        m_lastErrorInfo = "DecodeJsonData() UpdateModel Error.";
        return cfgErr;
    }

    m_dataIdx = data_idx;
    return SENetConfig::Error::OK;
}

int32_t SENetConfig::DecodeTFSModelFolder(
    const rapidjson::Document &doc,
    const int data_idx)
{
    if (!doc.HasMember("TFSModelFolder"))
    {
        return SENetConfig::Error::OK;
    }

    if (!doc["TFSModelFolder"].IsArray())
    {
        LOG(ERROR) << "DecodeTFSModelFolder() Parse jsonData TFSModelFolder Not Array.";
        return SENetConfig::Error::DecodeTFSModelFolderError;
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
            return SENetConfig::Error::DecodeTFSModelFolderError;
        }
        vecFolder.emplace_back(std::string(array[idx].GetString(), array[idx].GetStringLength()));
    }
    return SENetConfig::Error::OK;
}

int32_t SENetConfig::DecodeSENet(const rapidjson::Document &doc, const int data_idx)
{
    if (!doc.HasMember("SENet"))
    {
        return SENetConfig::Error::OK;
    }

    if (!doc["SENet"].IsArray())
    {
        LOG(ERROR) << "DecodeSENet() Parse jsonData SENet Not Array.";
        return SENetConfig::Error::DecodeSENetError;
    }

    auto &data = m_data[data_idx];
    auto array = doc["SENet"].GetArray();
    auto array_size = array.Size();
    data.vecSENetData.clear();
    data.vecSENetData.resize(array_size);
    for (rapidjson::SizeType idx = 0; idx < array_size; idx++)
    {
        if (!array[idx].IsObject())
        {
            LOG(ERROR) << "DecodeSENet() Parse jsonData"
                       << ", SENet Not Object"
                       << ", idx = " << idx;
            return SENetConfig::Error::DecodeSENetError;
        }

        auto item = array[idx].GetObject();
        auto &model_data = data.vecSENetData[idx];

        // 服务名称
        if (item.HasMember("ServiceName") && item["ServiceName"].IsString())
        {
            model_data.ServiceName = std::string(item["ServiceName"].GetString(),
                                                 item["ServiceName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeSENet() Parse jsonData"
                       << ", SENet Not Find ServiceName"
                       << ", idx = " << idx;
            return SENetConfig::Error::DecodeSENetError;
        }

        // 模型名称
        if (item.HasMember("ModelName") && item["ModelName"].IsString())
        {
            model_data.ModelName = std::string(item["ModelName"].GetString(),
                                               item["ModelName"].GetStringLength());
        }
        else
        {
            LOG(ERROR) << "DecodeSENet() Parse jsonData"
                       << ", SENet Not Find ModelName"
                       << ", idx = " << idx;
            return SENetConfig::Error::DecodeSENetError;
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
            auto &vecTFSAddrList = model_data.TFSAddrList;
            vecTFSAddrList.resize(tfs_addr_array_size);
            for (rapidjson::SizeType tfs_addr_idx = 0; tfs_addr_idx < tfs_addr_array_size; tfs_addr_idx++)
            {
                if (!tfs_addr_array[tfs_addr_idx].IsString())
                {
                    LOG(ERROR) << "DecodeSENet() Parse jsonData"
                               << ", TFSAddrList tfs_addr_idx = " << tfs_addr_idx << " Not String.";
                    return SENetConfig::Error::DecodeSENetError;
                }

                vecTFSAddrList[tfs_addr_idx] = std::string(tfs_addr_array[tfs_addr_idx].GetString(),
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
                LOG(ERROR) << "DecodeSENet() RequestMethod Not equal"
                           << ", method = " << method;
                return SENetConfig::Error::DecodeSENetError;
            }
        }

        // 模型签名
        if (item.HasMember("OutputName") && item["OutputName"].IsString())
        {
            model_data.OutputName = std::string(item["OutputName"].GetString(),
                                                item["OutputName"].GetStringLength());
        }

        // SENet模型返回Emb长度
        if (item.HasMember("EmbSize") && item["EmbSize"].IsInt())
        {
            model_data.EmbSize = item["EmbSize"].GetInt();
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
                LOG(ERROR) << "DecodeSENet() model_data.RequestMethod Not equal"
                           << ", model_data.RequestMethod = " << model_data.RequestMethod;
                return SENetConfig::Error::DecodeSENetError;
            }
        }
    }
    return SENetConfig::Error::OK;
}

int32_t SENetConfig::UpdateModel(const int dataIdx)
{
    static const std::string latest = "latest"; // 最新版本号

    auto &data = m_data[dataIdx];

    // 更新SENet
    for (auto &model : data.vecSENetData)
    {
        // 先查找 model_format 文件
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
                LOG(ERROR) << "ModelConfig::UpdateModel() format_filePath Not directory"
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
            LOG(ERROR) << "ModelConfig::UpdateModel() SENet File Not Found"
                       << ", model.ServiceName = " << model.ServiceName
                       << ", model.ModelName = " << model.ModelName
                       << ", model.Version = " << model.Version
                       << ", model.TransFileName = " << model.TransFileName;
            return SENetConfig::Error::UpdateModelError;
        }

        // 找到 SENet
        std::shared_ptr<SENet> spModel = nullptr;
        {
            auto it = data.mapSENet.find(model.ModelName);
            if (it != data.mapSENet.end())
            {
                spModel = it->second;
            }
            if (spModel == nullptr)
            {
                spModel = std::make_shared<SENet>(model.ModelName);
                data.mapSENet[model.ModelName] = spModel;
            }
        }

        // 更新 SENet
        const int version = (model.Version == latest) ? 0 : atoi(model.Version.c_str());
        if (!spModel->Update(model.TFSAddrList, model.SignatureName,
                             model.Timeout, trans_filePath,
                             model.UseDropoutKeep, version, model.EmbSize,
                             model.RequestMethod, model.OutputName))
        {
            LOG(ERROR) << "ModelConfig::UpdateModel() SENet Update Failed"
                       << ", model.ServiceName = " << model.ServiceName
                       << ", model.ModelName = " << model.ModelName
                       << ", version = " << version
                       << ", trans_filePath = " << trans_filePath;
            return SENetConfig::Error::UpdateModelError;
        }
        else
        {
            LOG(INFO) << "ModelConfig::UpdateModel() SENet Update Succ"
                      << ", model.ServiceName = " << model.ServiceName
                      << ", model.ModelName = " << model.ModelName
                      << ", version = " << version
                      << ", trans_filePath = " << trans_filePath;

            // 记录模型名称对应文件地址
            data.mapSENet2TransFilePath[model.ModelName] = trans_filePath;
        }
    }
    return SENetConfig::Error::OK;
}
