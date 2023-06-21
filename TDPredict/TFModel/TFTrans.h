#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include "Common/Function.h"
#include "glog/logging.h"
#include "../Interface/ModelInterface.h"

namespace TDPredict
{
    struct FieldFormat
    {
        int field_begin; // field起始维度下标
        int field_count; // field使用维度数量
        FieldFormat(int fb, int fc) : field_begin(fb), field_count(fc) {}
    };

    using TFTransFormat = std::unordered_map<int, FieldFormat>;

    // TF转换数据
    struct TFTransData
    {
        int all_field_count;                         // 总特征维度
        TFTransFormat field_trans;                   // TF域转换
        std::unordered_map<long, int> trans_mapping; // 转换TF特征值

        void Clear()
        {
            all_field_count = 0;
            field_trans.clear();
            trans_mapping.clear();
        }

        // 加载TF转换数据
        bool LoadTransFile(const std::string &trans_file_path)
        {
            std::string buffer;
            if (true == Common::FastReadFile(trans_file_path, buffer, false))
            {
                return LoadTransBuffer(buffer);
            }
            else
            {
                LOG(ERROR) << "TFTransData::LoadTransFile() FastReadFile Failed"
                           << ", trans_file_path = " << trans_file_path;
            }
            return false;
        }

        bool LoadTransBuffer(const std::string &buffer)
        {
            Clear();

            std::string line;
            std::stringstream ss(buffer);
            if (!getline(ss, line, '\n'))
            {
                LOG(ERROR) << "TFTransData::LoadTransBuffer() Failed, getline line = " << line;
                return false;
            }

            // 第一行是一个数字, 标识最大特征维度
            all_field_count = atoi(line.c_str());

            int cur_index = 0;
            field_trans.clear();
            trans_mapping.clear();
            while (getline(ss, line, '\n'))
            {
                std::vector<std::string> str_vec;
                Common::SplitString(line, ' ', str_vec);
                if (str_vec.size() == 2)
                {
                    long field = std::atol(str_vec[0].c_str());
                    int count = std::atoi(str_vec[1].c_str());
                    if (0 < field && field < FIELD_MAX)
                    {
                        field_trans.emplace(field, FieldFormat(cur_index, count));
                        cur_index += count;
                    }
                    else
                    {
                        // P.s> 等于FIELD_MAX的情况, 等于可认为是触碰临界, 判定无效
                        LOG(ERROR) << "TFTransData::LoadTransBuffer() Failed, field invalid"
                                   << ", field = " << field;
                        return false;
                    }
                }
                else if (str_vec.size() == 3)
                {
                    // long field_8_value_24 = std::atol(str_vec[0].c_str());
                    int field = std::atoi(str_vec[0].c_str());
                    int value = std::atoi(str_vec[1].c_str());
                    int index = std::atoi(str_vec[2].c_str());
                    if (0 < field && field < FIELD_MAX)
                    {
                        FieldValue fv(field, value);
                        trans_mapping[fv.field_value()] = index;
                    }
                    else
                    {
                        LOG(ERROR) << "TFTransData::LoadTransBuffer() Failed, field invalid"
                                   << ", field = " << field;
                        return false;
                    }
                }
                else
                {
                    LOG(ERROR) << "TFTransData::LoadTransBuffer() Failed"
                               << ", str_vec.size() = " << str_vec.size()
                               << ", line = " << line;
                    return false;
                }
            }

            // 最后校验一次.
            if (cur_index != all_field_count)
            {
                LOG(ERROR) << "TFTransData::LoadTransBuffer() Failed"
                           << ", cur_index = " << cur_index
                           << ", all_field_count = " << all_field_count;
                return false;
            }

            return true;
        }
    };

} // namespace TDPredict
