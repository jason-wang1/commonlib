#include "ProtoFactory.h"
#include <filesystem>
#include <regex>

bool ProtoFactory::Init(const std::string &proto_path)
{
    std::filesystem::path _root(proto_path);
    if (!std::filesystem::exists(_root))
    {
        return false;
    }

    bool add_proto = false;
    std::regex fileSuffix("(.*).(.proto)");
    if (std::filesystem::is_directory(_root))
    {
        // 传入是文件夹, 设置根目录, 遍历目录下所有proto文件并进行加载.
        m_st.MapPath("", _root.string());
        for (auto &_iter : std::filesystem::directory_iterator(_root))
        {
            auto filepath = _iter.path();
            if (std::filesystem::is_regular_file(filepath))
            {
                auto filename = filepath.filename().string();
                if (std::regex_match(filename, fileSuffix))
                {
                    auto ptr = m_des_pool.FindFileByName(filename);
                    if (ptr != nullptr)
                    {
                        add_proto = true;
                    }
                }
            }
        }
    }
    else if (std::filesystem::is_regular_file(_root))
    {
        // 传入是proto文件路径, 设置其文件夹路径为根目录, 再加载文件.
        // m_st.MapPath("", _root.parent_path().string());

        // 传入是proto文件路径, 直接加载其文件路径
        const std::string filename = _root.filename().string();
        m_st.MapPath(filename, _root.string());
        if (std::regex_match(filename, fileSuffix))
        {
            auto ptr = m_des_pool.FindFileByName(filename);
            if (ptr != nullptr)
            {
                add_proto = true;
            }
        }
    }
    return add_proto;
}

SharedPtrProto ProtoFactory::GetProtoMessage(const std::string &proto_name) noexcept
{
    // 通过名称生成新对象
    auto _des = m_des_pool.FindMessageTypeByName(proto_name);
    if (_des == nullptr)
    {
        return nullptr;
    }
    auto _proto_type = m_dynamic_factory.GetPrototype(_des);
    if (_proto_type == nullptr)
    {
        return nullptr;
    }
    return SharedPtrProto(_proto_type->New(), ProtoDeleter());
}
