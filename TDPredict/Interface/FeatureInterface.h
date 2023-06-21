#pragma once
#include <string>
#include <any>
#include "ModelInterface.h"
#include "Common/CommonFactory.h"

// 注册特征, 通过类静态成员变量在main开始前的构造完成注册
#define RegisterFeature(ClassName)                          \
    inline static const TDPredict::FeatureFactory::Register \
        _TDPredict_FeatureRegister_##ClassName##_ = {       \
            #ClassName,                                     \
            std::make_shared<TDPredict::FeatureBuilder<ClassName>>()};

namespace TDPredict
{
    // FeatureInterafce 特征公共接口
    // 1. 匹配特征模型
    // 2. 拼接特征
    // 3. 返回拼接结果
    class FeatureInterafce
    {
    public:
        // 初始化数据接口
        virtual bool SetInitData(const std::any init_data) noexcept = 0;

        // 公共特征, 比如用户特征
        virtual bool AnalyseCommonFeature(TDPredict::RankItem &rank_item) const noexcept = 0;

        // 排序特征, 比如地图特征、房主特征、Query特征等...
        virtual bool AnalyseRankFeature(TDPredict::RankItem &rank_item) const noexcept = 0;
    };

    // 特征模型工厂
    using FeatureFactory = CommonFactory<FeatureInterafce>;
    using FeatureFactoryPtr = std::shared_ptr<FeatureFactory>;
    inline FeatureFactoryPtr &GetFeatureFactory()
    {
        return FeatureFactory::GetInstance();
    }

    // 特征模型生成器模板
    template <class T>
    class FeatureBuilder final : public CommonBuilder_Interface<FeatureInterafce>
    {
    public:
        virtual std::shared_ptr<FeatureInterafce> build() const override
        {
            return std::make_shared<T>();
        }
    };

} // namespace TDPredict
