#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <unordered_map>

using score_type = float;
using VecRankFeature = std::vector<std::pair<int, score_type>>;

//把模型数据抽象出来，实现双buffer
struct FMModelData
{
    int m_numFactor;
    score_type m_w0;
    std::unordered_map<int, score_type> m_w1;
    std::unordered_map<int, std::vector<score_type>> m_w2;
};

struct DBufFMModelData
{
public:
    DBufFMModelData() : m_dataIdx(0) {}
    bool Init(int num_factor, const std::string &model_file_path);

public:
    const FMModelData &GetCurModelData() const noexcept
    {
        return m_modelData[m_dataIdx];
    }

public:
    //加载模型文件
    bool LoadModelFile(int num_factor, const std::string &model_file_path);
    bool LoadModelBuffer(int num_factor, const std::string &buffer);

private:
    FMModelData m_modelData[2];
    std::atomic<int> m_dataIdx;
};

class FMModel final
{
public:
    FMModel() {}
    FMModel(const std::string &model_name)
    {
        m_fmModelName = model_name;
    }
    ~FMModel() {}

public:
    // 初始化函数
    // [in] num_factor: 模型位数
    // [in] model_path: 模型文件地址
    virtual bool Init(int num_factor, const std::string &model_path); //初始化函数

    // 更新模型文件
    // [in] num_factor: 模型位数
    // [in] model_path: 模型文件地址
    virtual bool Update(int num_factor, const std::string &model_path);

    // 热更新函数
    // [in] num_factor: 模型位数
    // [in] buffer: 模型文件内容
    virtual bool UpdateModel(int num_factor, const std::string &buffer);

    // 热更新函数
    // [in] buffer: 模型文件内容
    virtual bool UpdateModel(const std::string &buffer);

    // 预测函数
    virtual score_type predict(const VecRankFeature &x) const noexcept;

    virtual std::string GetName() const noexcept
    {
        return m_fmModelName;
    }

private:
    std::string m_fmModelName;
    DBufFMModelData m_fmModelData;
};