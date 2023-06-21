#include "FMModel.h"
#include <cmath>
#include "Common/Function.h"
#include "glog/logging.h"

bool DBufFMModelData::Init(int num_factor, const std::string &model_file_path)
{
    m_dataIdx = 1;
    m_modelData[0].m_numFactor = num_factor;
    m_modelData[1].m_numFactor = num_factor;
    return LoadModelFile(num_factor, model_file_path);
}

//加载模型文件
bool DBufFMModelData::LoadModelFile(int num_factor, const std::string &model_file_path)
{
    std::string buffer;
    if (true == Common::FastReadFile(model_file_path, buffer, false))
    {
        return LoadModelBuffer(num_factor, buffer);
    }
    else
    {
        LOG(ERROR) << "DBufFMModelData::LoadModelFile FastReadFile Failed"
                   << ", num_factor = " << num_factor
                   << ", model_file_path = " << model_file_path;
    }
    return false;
}

bool DBufFMModelData::LoadModelBuffer(int num_factor, const std::string &buffer)
{
    int dataIdx = (m_dataIdx + 1) % 2;
    auto &data = m_modelData[dataIdx];

    std::string line;
    std::stringstream ss(buffer);

    if (!getline(ss, line, '\n'))
    {
        LOG(ERROR) << "DBufFMModelData::LoadModelBuffer Failed, getline line = " << line;
        return false;
    }

    size_t tmpSize = (3 * num_factor + 4);
    std::vector<std::string> strVec;
    strVec.reserve(tmpSize);

    Common::SplitString(line, ' ', strVec);
    if (strVec.size() != 4)
    {
        LOG(ERROR) << "DBufFMModelData::LoadModelBuffer Failed, getline strVec.size() != 4, strVec.size() = " << strVec.size();
        return false;
    }

    data.m_numFactor = num_factor;
    data.m_w0 = std::atof(strVec[1].c_str());

    // 清理一下数据, 不然可能会有上次留下来的脏数据
    data.m_w1.clear();
    data.m_w2.clear();
    while (getline(ss, line, '\n'))
    {
        strVec.clear();
        Common::SplitString(line, ' ', strVec);
        if (strVec.size() != tmpSize)
        {
            LOG(ERROR) << "DBufFMModelData::LoadModelBuffer Failed, getline strVec.size() != tmpSize"
                       << ", strVec.size() = " << strVec.size()
                       << ", tmpSize = " << tmpSize;
            return false;
        }

        int index = std::atoi(strVec[0].c_str());
        data.m_w1[index] = std::atof(strVec[1].c_str());

        std::vector<score_type> tmp_w2_value;
        tmp_w2_value.reserve(num_factor);
        for (int i = 2, end = num_factor + 2; i < end; i++)
        {
            tmp_w2_value.emplace_back(std::atof(strVec[i].c_str()));
        }
        data.m_w2[index] = std::move(tmp_w2_value);
    }

    m_dataIdx = dataIdx;
    return true;
}

// 初始化函数
// [in] num_factor: 模型位数
// [in] model_path: 模型文件地址
bool FMModel::Init(int num_factor, const std::string &model_path)
{
    return m_fmModelData.Init(num_factor, model_path);
}

// 更新函数
// [in] num_factor: 模型位数
// [in] model_path: 模型文件地址
bool FMModel::Update(int num_factor, const std::string &model_path)
{
    return m_fmModelData.LoadModelFile(num_factor, model_path);
}

// 热更新函数
// [in] num_factor: 模型位数
// [in] buffer: 模型文件内容
bool FMModel::UpdateModel(int num_factor, const std::string &buffer)
{
    return m_fmModelData.LoadModelBuffer(num_factor, buffer);
}

// 热更新函数
// [in] buffer: 模型文件内容
bool FMModel::UpdateModel(const std::string &buffer)
{
    const auto &model_data = m_fmModelData.GetCurModelData();
    return m_fmModelData.LoadModelBuffer(model_data.m_numFactor, buffer);
}

// 预测函数(计算函数)
score_type FMModel::predict(
    const VecRankFeature &x) const noexcept
{
    // 确定辅助空间大小
    const auto &model_data = m_fmModelData.GetCurModelData();
    const int num_factor = model_data.m_numFactor;
    std::vector<score_type> tmp_vec_sum(num_factor, 0.0);
    std::vector<score_type> tmp_vec_sum_sqr(num_factor, 0.0);

    // 计算中间变量
    score_type result = model_data.m_w0;
    const auto iter_w1_end = model_data.m_w1.end();
    const auto iter_w2_end = model_data.m_w2.end();
    for (const auto &pr : x)
    {
        const auto iter_w1 = model_data.m_w1.find(pr.first);
        if (iter_w1 != iter_w1_end)
        {
            result += iter_w1->second * pr.second;
        }

        const auto iter_w2 = model_data.m_w2.find(pr.first);
        if (iter_w2 != iter_w2_end)
        {
            const auto &vec_w2 = iter_w2->second;
            for (int factor = 0; factor < num_factor; ++factor)
            {
                score_type d = vec_w2[factor] * pr.second;
                tmp_vec_sum[factor] += d;
                tmp_vec_sum_sqr[factor] += (d * d);
            }
        }
    }

    // 计算最终结果
    auto tmp_result = 0.0;
    for (int factor = 0; factor < num_factor; ++factor)
    {
        const auto &tmp_sum = tmp_vec_sum[factor];
        tmp_result += (tmp_sum * tmp_sum - tmp_vec_sum_sqr[factor]);
    }
    result += (tmp_result * 0.5);

    result = exp(-result);
    return 1.0 / (1.0 + result);
}
