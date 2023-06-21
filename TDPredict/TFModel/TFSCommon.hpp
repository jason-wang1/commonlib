#pragma once
#include "TFTrans.h"
#include "TFModelBase.h"
#include "../Interface/ModelInterface.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace TDPredict
{
    inline void TransTFSFeature(
        const TDPredict::FeatureItem &feature_item,
        const TFTransData &trans_data,
        TDPredict::TFSInputData &tfs_input) noexcept
    {
        if (feature_item.size() != FIELD_MAX)
        {
            return;
        }

        for (const auto &pr_trans : trans_data.field_trans)
        {
            const int trans_field = pr_trans.first;
            const int field_begin = pr_trans.second.field_begin;
            const int field_count = pr_trans.second.field_count;

            auto &field_item_list = feature_item[trans_field];
            if (field_count == 0 || field_item_list.empty())
            {
                continue;
            }

            const int field_item_list_size = field_item_list.size();
            int use_count = std::min(field_count, field_item_list_size);
            if (use_count == 0)
            {
                continue;
            }

            // 两种特征填充方式:
            if (field_count >= field_item_list_size)
            {
                // 1. 当需求特征数量大于/等于当前特征数量时, 不考虑补充逻辑, 按顺序转换完成特征保序
                for (int idx = 0; idx < use_count; idx++)
                {
                    auto &field_item = field_item_list[idx];
                    auto iter_tm = trans_data.trans_mapping.find(field_item.field_value.field_value());
                    if (iter_tm != trans_data.trans_mapping.end())
                    {
                        tfs_input.index[field_begin + idx] = iter_tm->second;
                        tfs_input.value[field_begin + idx] = field_item.weight;
                    }
                }
            }
            else
            {
                // 2. 当需求的特征数量小于当前特征数量时, 启动补充逻辑, 允许后面的有效特征补齐前面的无效特征
                int push_count = 0;
                for (auto &field_item : field_item_list)
                {
                    auto iter_tm = trans_data.trans_mapping.find(field_item.field_value.field_value());
                    if (iter_tm != trans_data.trans_mapping.end())
                    {
                        tfs_input.index[field_begin + push_count] = iter_tm->second;
                        tfs_input.value[field_begin + push_count] = field_item.weight;
                        push_count += 1;
                        if (push_count >= use_count)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    inline bool get_tfs_result(
        const std::string &tfs_result,
        TDPredict::score_type &tfs_result_score) noexcept
    {
        if (tfs_result.empty())
        {
            return false;
        }

        rapidjson::Document doc;
        if (doc.Parse(tfs_result.data()).HasParseError())
        {
            return false;
        }

        // 结果不存在
        if (!doc.HasMember("result_value"))
        {
            return false;
        }

        // 支持以下两种形式:
        // 1. 从Double数组中取出第0项的值作为结果返回
        // 2. 直接获取Double值作为结果返回
        if (doc["result_value"].IsArray())
        {
            // 取Item第0项double值, 获取最终结果
            auto result_array = doc["result_value"].GetArray();
            if (result_array.Size() < 1)
            {
                return false;
            }
            if (!result_array[0].IsDouble())
            {
                return false;
            }
            tfs_result_score = result_array[0].GetDouble();
        }
        else if (doc["result_value"].IsDouble())
        {
            tfs_result_score = doc["result_value"].GetDouble();
        }
        else
        {
            return false;
        }
        return true;
    }

    inline bool get_tfs_result(
        const std::string &tfs_result,
        std::vector<score_type> &tfs_result_vector) noexcept
    {
        if (tfs_result.empty())
        {
            return false;
        }

        rapidjson::Document doc;
        if (doc.Parse(tfs_result.data()).HasParseError())
        {
            return false;
        }

        // 结果不存在
        if (!doc.HasMember("result_value"))
        {
            return false;
        }

        // 支持以下两种形式:
        // 1. 从Double数组中取出第0项的值作为结果返回
        // 2. 直接获取Double值作为结果返回
        if (doc["result_value"].IsArray())
        {
            // 取Item第0项double值, 获取最终结果
            auto result_array = doc["result_value"].GetArray();
            const int array_size = result_array.Size();
            tfs_result_vector.resize(array_size, 0.0);
            for (int idx = 0; idx < array_size; idx++)
            {
                if (result_array[idx].IsDouble())
                {
                    tfs_result_vector[idx] = result_array[idx].GetDouble();
                }
                else
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
        return true;
    }

}