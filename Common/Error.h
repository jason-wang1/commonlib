#pragma once
#include <ostream>

namespace Common
{
    // 错误码 - 全模块通用
    enum Error
    {
        OK = 0,
        Failed = 100,

        Recommend_Error = 10000,         // 推荐服务错误
        Recommend_InvalidParams = 10001, // 推荐服务参数错误

        // RedisProtoData
        RPD_Error = 20000,                // 获取Redis数据
        RPD_GetConnFailed = 20001,        // 获取连接失败
        RPD_RequestFailed = 20002,        // 请求失败
        RPD_ParamCheckError = 20003,      // 参数校验失败
        RPD_ConfigInitError = 20004,      // 配置文件加载出错
        RPD_GetProtoFailed = 20005,       // 获取Proto失败
        RPD_GetProtoDataEmpty = 20006,    // 获取Proto数据为空
        RPD_ProtoParseFailed = 20007,     // Proto解析失败
        RPD_ProtoSerializeFailed = 20008, // Proto序列化失败

        // RedisProtoData Cache
        RPDCache_InvalidCacheType = 20010, // 没有这个类型的缓存结构
        RPDCache_CacheIsEmpty = 20011,     // 缓存结构为空
        RPDCache_DataNotFind = 20012,      // 无对应缓存Proto数据
        RPDCache_NotInit = 20013,          // 没有初始化
        RPDCache_RepeatedInit = 20014,     // 重复初始化

        // 召回
        Recall_Error = 30000,                  // 召回错误
        Recall_GetRecallInstError = 30001,     // 获取召回实例出错
        Recall_APITypeNotSupport = 30002,      // 实例不支持的APIType
        Recall_RecallTypeNotSupport = 30003,   // 实例不支持的召回类型
        Recall_GetParamsError = 30004,         // 获取召回策略参数出错
        Recall_RecallCalFailed = 30005,        // 召回计算出错
        Recall_GetProtoDataFailed = 30006,     // 获取Proto数据失败
        Recall_ParseProtoDataFailed = 30007,   // 解码Proto数据失败
        Recall_EncodeRequestFailed = 30008,    // 编码请求信息失败
        Recall_DecodeResponseFailed = 30009,   // 解码返回信息失败
        Recall_CallRecallServerFailed = 30010, // 调用召回服务失败
        Recall_RecallEmptyError = 30011,       // 召回数据空错误
        Recall_RecallCalcFailed = 30012,       // 召回计算错误

        // 召回 Annoy检索
        Recall_Annoy_Error = 31000,                // 检索错误
        Recall_Annoy_DataIsEmpty = 31001,          // 空数据
        Recall_Annoy_NotInit = 31002,              // 没有初始化
        Recall_Annoy_DataError = 31003,            // 数据错误
        Recall_Annoy_ParamError = 31004,           // 参数错误
        Recall_Annoy_ParseProtoDataFailed = 31005, // 解码Proto数据失败

        // 过滤
        Filter_Error = 40000,                // 过滤错误
        Filter_MergeFailed = 40001,          // 合并计算错误
        Filter_FilterFailed = 40002,         // 过滤计算错误
        Filter_FilterExposureFailed = 40004, // 曝光过滤计算错误
        Filter_GetStrategyInstError = 40005, // 获取过滤策略实例出错
        Filter_GetParamsError = 40006,       // 获取过滤策略参数出错
        Filter_ParseProtoDataFailed = 40007, // 解码Proto数据失败
        Filter_GetCacheDataFailed = 40008,   // 获取缓存数据失败

        // 排序
        Rank_Error = 50000,   // 排序错误
        Rank_Timeout = 50001, // 排序超时

        // 展控
        Display_Error = 60000,                // 展控模块出错
        Display_GetStrategyInstError = 60001, // 获取展控策略实例出错
        Display_GetParamsError = 60002,       // 获取展控策略参数出错
        Display_ParseProtoDataFailed = 60003, // 解码Proto数据失败

    };

    inline std::ostream &operator<<(std::ostream &out, Common::Error err)
    {
        out << static_cast<std::underlying_type<Common::Error>::type>(err);
        return out;
    }
}