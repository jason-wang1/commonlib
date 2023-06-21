#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <unordered_set>
#include <unordered_map>

#include "glog/logging.h"
#include "Common/Function.h"
#include "../Interface/ModelInterface.h"

namespace TDPredict
{
    using FMTransFormat = std::unordered_map<int, int>;

    // FM输入格式数据
    struct FMTransData
    {
        int all_field_count;       // 总特征维度
        FMTransFormat field_trans; // FM特征域转换

        void Clear()
        {
            field_trans.clear();
        }

        // 加载FM转换数据
        bool LoadTransFile(const std::string &format_file_path)
        {
            std::string buffer;
            if (true == Common::FastReadFile(format_file_path, buffer, false))
            {
                return LoadTransBuffer(buffer);
            }
            else
            {
                LOG(ERROR) << "FMTransData::LoadTransBuffer() FastReadFile Failed"
                           << ", format_file_path = " << format_file_path;
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
                LOG(ERROR) << "FMTransData::LoadTransBuffer() Failed, getline line = " << line;
                return false;
            }

            // 第一行是一个数字, 标识最大特征维度
            all_field_count = atoi(line.c_str());

            int cur_index = 0;
            field_trans.clear();
            std::vector<std::string> str_vec;
            while (getline(ss, line, '\n'))
            {
                str_vec.clear();
                Common::SplitString(line, ' ', str_vec);
                if (str_vec.size() != 2)
                {
                    LOG(ERROR) << "FMTransData::LoadTransBuffer() Failed"
                               << ", str_vec.size() = " << str_vec.size()
                               << ", line = " << line;
                    return false;
                }

                const int field = std::atoi(str_vec[0].c_str());
                const int count = std::atoi(str_vec[1].c_str());
                if (0 < field && field < FIELD_MAX)
                {
                    field_trans[field] = count;
                    cur_index += count;
                }
                else
                {
                    LOG(ERROR) << "FMTransData::LoadTransBuffer() check field Failed"
                               << ", field = " << field
                               << ", count = " << count;
                    return false;
                }
            }

            // 最后校验一次.
            if (cur_index != all_field_count)
            {
                LOG(ERROR) << "FMTransData::LoadTransBuffer() Failed"
                           << ", cur_index = " << cur_index
                           << ", all_field_count = " << all_field_count;
                return false;
            }

            return true;
        }
    };

} // namespace TDPredict
