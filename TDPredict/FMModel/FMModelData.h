#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../Interface/FieldValue.h"
#include "../Interface/ModelInterface.h"
#include "glog/logging.h"
#include "Common/Function.h"

namespace TDPredict
{
    struct FMModelData
    {
        int m_factor;
        score_type m_w0;
        std::unordered_map<long, score_type> m_w1;
        std::unordered_map<long, std::vector<score_type>> m_w2;

        void Clear()
        {
            m_factor = 0;
            m_w0 = 0;
            m_w1.clear();
            m_w2.clear();
        }

        bool LoadModelFile(int factor, const std::string &model_file_path)
        {
            std::string buffer;
            if (!Common::FastReadFile(model_file_path, buffer, false))
            {
                LOG(ERROR) << "FMModelData::LoadModelFile() FastReadFile Failed"
                           << ", factor = " << factor
                           << ", model_file_path = " << model_file_path;
                return false;
            }
            return LoadModelBuffer(factor, buffer);
        }

        //加载模型
        bool LoadModelBuffer(int factor, const std::string &buffer)
        {
            Clear();

            std::string line;
            std::stringstream ss(buffer);
            if (!getline(ss, line, '\n'))
            {
                LOG(ERROR) << "FMModelData::LoadModelBuffer() Failed, getline line = " << line;
                return false;
            }

            std::vector<std::string> strVec;
            Common::SplitString(line, ' ', strVec);
            if (strVec.size() != 4)
            {
                LOG(ERROR) << "FMModelData::LoadModelBuffer() Failed, getline strVec.size() != 4"
                           << ", strVec.size() = " << strVec.size()
                           << ", line = " << line;
                return false;
            }
            m_w0 = std::atof(strVec[1].c_str());
            m_factor = factor;

            size_t validSize = (3 * m_factor + 4);
            strVec.reserve(validSize);
            while (getline(ss, line, '\n'))
            {
                strVec.clear();
                Common::SplitString(line, ' ', strVec);
                if (strVec.size() != validSize)
                {
                    LOG(ERROR) << "FMModelData::LoadModelBuffer() Failed, getline strVec.size() != validSize"
                               << ", strVec.size() = " << strVec.size()
                               << ", validSize = " << validSize
                               << ", line = " << line;
                    return false;
                }

                FieldValue fv;
                {
                    const long index = std::atol(strVec[0].c_str());
                    const int field = index >> 32;
                    if (field == 0)
                    {
                        // 前32位都是0, 对应旧方案[32][8][24], 中间8位是field, 后24位是value
                        fv.set_feild_value((index >> 24) & 0xFF, index & 0xFFFFFF);
                    }
                    else if (0 < field && field < FIELD_MAX)
                    {
                        // 新field有效, 新方案[32][32], 此时Index不需要做任何修改
                        fv.set_feild_value(index);
                    }
                    else
                    {
                        LOG(ERROR) << "FMModelData::LoadModelBuffer() Failed, field invalid"
                                   << ", index = " << index
                                   << ", field = " << field
                                   << ", line = " << line;
                        return false;
                    }
                }

                m_w1[fv.field_value()] = std::atof(strVec[1].c_str());
                std::vector<score_type> tmp_w2_value;
                tmp_w2_value.reserve(m_factor);
                for (int i = 2, end = m_factor + 2; i < end; i++)
                {
                    tmp_w2_value.emplace_back(std::atof(strVec[i].c_str()));
                }
                m_w2[fv.field_value()] = std::move(tmp_w2_value);
            }
            return true;
        }
    };
}