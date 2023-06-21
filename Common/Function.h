#pragma once
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <algorithm>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <cstring>
#include <strings.h>
#include <unistd.h>
#include <pwd.h>
#include <filesystem>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include "Time.h"

namespace Common
{
    // IsFile 使用 std::filesystem::is_regular_file(filePath) 替换
    // IsDir  使用 std::filesystem::is_directory(dirPath) 替换
    // GetFileName std::filesystem::path(filePath).filename().string()

    // 快速读取文件内容
    // [in] filePath, 文件路径
    // [out] fileData, 文件内容
    // [in, param] lock, 读取文件时加共享锁, 加锁失败返回失败
    // [ret] 返回成功or失败
    inline bool FastReadFile(
        const std::string &filePath,
        std::string &fileData,
        bool lock)
    {
        // buf的大小推荐4k或者1k都行, 一次性全读虽然快, 但是浪费内存
        static constexpr long bufSize = 4096;

        // 判断给定的路径是一个文件
        if (!std::filesystem::is_regular_file(filePath))
        {
            return false;
        }

        // 采用C方式读取文件信息, 比CPP的readline方式快N倍.
        FILE *pFile;
        if ((pFile = fopen(filePath.c_str(), "r")) == NULL)
        {
            return false;
        }

        // 共享锁/不阻塞
        if (lock && flock(fileno(pFile), LOCK_SH | LOCK_NB) != 0)
        {
            fclose(pFile);
            return false;
        }

        // 计算文件大小
        fseek(pFile, 0, SEEK_SET);
        long begin = ftell(pFile);
        fseek(pFile, 0, SEEK_END);
        long end = ftell(pFile);
        long fileSize = end - begin;
        fseek(pFile, 0, SEEK_SET); // 重新指向文件头

        // 预分配内存空间
        fileData.reserve(fileSize + 1);

        // 读取文件内容
        char readBuf[bufSize + 1];
        long readSize = 0;
        while (readSize < fileSize)
        {
            long minRead = std::min(fileSize - readSize, bufSize);
            long len = fread(readBuf, 1, minRead, pFile);
            readSize += len;
            fileData.append(readBuf, len);
        }

        // 解锁
        if (lock && flock(fileno(pFile), LOCK_UN) != 0)
        {
            fclose(pFile);
            return false;
        }

        fclose(pFile);
        return true;
    }

    // 读取文件内容, 采用UNIX系统调用的方案
    // 解决部分情况下, 系统相关文件读取不了的问题
    inline bool SystemCall_ReadFile(
        const std::string &fileName,
        std::string &fileData)
    {
        if (access(fileName.c_str(), F_OK) == -1)
        {
            return false;
        }

        int fd = -1;
        if ((fd = open(fileName.c_str(), O_RDONLY)) == -1)
        {
            return false;
        }

        fileData.clear();
        char buffer[1024 + 1];
        memset(buffer, 0x00, sizeof(buffer));
        while (read(fd, (void *)buffer, 1024) > 0)
        {
            fileData += std::string(buffer, strlen(buffer));
            memset(buffer, 0x00, sizeof(buffer));
        }

        close(fd);
        return true;
    }

    // 获取目录文件数量, 采用UNIX系统调用的方案
    inline bool SystemCall_GetFileNumInDir(
        const std::string &dirPath,
        int &fileNum)
    {
        if (access(dirPath.c_str(), F_OK) == -1)
        {
            return false;
        }

        DIR *dirPtr = opendir(dirPath.c_str());
        if (dirPtr == NULL)
        {
            return false;
        }

        fileNum = 0;
        struct dirent *direntPtr;
        while ((direntPtr = readdir(dirPtr)) != NULL)
        {
            if (strcmp(direntPtr->d_name, ".") == 0 ||
                strcmp(direntPtr->d_name, "..") == 0)
            {
                continue;
            }

            fileNum++;
        }

        closedir(dirPtr);
        return true;
    }

    // 写入文件内容
    // [in] filePath, 文件路径
    // [in] fileData, 写入内容
    // [in, param] lock, 写入文件时加互斥锁, 加锁失败返回失败
    // [ret] 返回写入成功or失败
    inline bool WriteFile(
        const std::string &filePath,
        const std::string &fileData,
        bool lock)
    {
        FILE *pFile;
        if ((pFile = fopen(filePath.c_str(), "w")) == NULL)
        {
            return false;
        }

        // 互斥锁/不阻塞
        if (lock && flock(fileno(pFile), LOCK_EX | LOCK_NB) != 0)
        {
            fclose(pFile);
            return false;
        }

        fwrite(fileData.c_str(), 1, fileData.length(), pFile);

        // 解锁
        if (lock && flock(fileno(pFile), LOCK_UN) != 0)
        {
            fclose(pFile);
            return false;
        }

        fclose(pFile);
        return true;
    }

    // 创建文件夹
    // [in] folderPath, 文件夹路径
    // [in] mode, 创建文件夹权限, 建议0755
    // [out] failed_info, 创建失败时返回错误信息
    // [ret] 创建成功or失败
    inline bool MkdirPath(
        const std::string &folderPath,
        const unsigned int mode,
        std::string &failed_info) noexcept
    {
        if (folderPath.empty())
        {
            return false;
        }

        // 已知: 这段代码会在权限不足的情况下产生异常
        // emmm... 需要 try-cache
        try
        {
            std::filesystem::path folder = folderPath;
            if (std::filesystem::is_directory(folder) ||
                std::filesystem::create_directories(folder))
            {
                std::filesystem::perms perm = std::filesystem::perms(mode);
                std::filesystem::permissions(folder, perm);
                return true;
            }
        }
        catch (const std::exception &e)
        {
            failed_info = std::string(e.what());
            return false;
        }
        return false;
    }

    inline std::string get_hostname() noexcept
    {
        char hostname[1024];
        if (::gethostname(hostname, sizeof(hostname)))
        {
            return {};
        }
        return hostname;
    }

    inline long get_cpu_core()
    {
        return sysconf(_SC_NPROCESSORS_ONLN);
    }

    inline std::string get_username()
    {
        const auto *pwd = getpwuid(getuid());
        return (pwd != nullptr) ? pwd->pw_name : "";
    }

    // 模板实现, 所有类型stl容器都可用, 遍历容器内容, 转到string中, 用char分隔
    template <typename T, template <class, class...> class C, class... Args>
    inline void ConvertToString(
        std::string &result,
        const C<T, Args...> &contain,
        std::string split = " ")
    {
        result.clear();
        if (contain.empty())
        {
            return;
        }

        // std::stringstream ss; //单字符集
        // std::wstringstream wss; // 宽字符集
        std::stringstream ss;
        ss.str("");

        // 遍历容器, 传值进流中
        for (auto &item : contain)
        {
            ss << item;
            ss << split;
        }
        result = ss.str();
    }

    // 针对std::vecotr<string>类型偏特化, 因为它不需要ss转化, 可以直接append.
    // P.S> 具体效率是否优化待测试, 预期是因为不需要转换, 能更快一些
    template <class... Args>
    inline void ConvertToString(
        std::string &result,
        const std::vector<std::string, Args...> &contain,
        std::string split = " ")
    {
        result.clear();
        if (contain.empty())
        {
            return;
        }

        // 假定每个string的大小和contain[0]相等, 计算大小
        // 注意每个string之间应该有split, 预申请空间
        result.reserve(contain.size() * (contain[0].size() + split.size()));

        // 遍历容器, 直接append进去就ok了...
        for (auto &item : contain)
        {
            result.append(item);
            result.append(split);
        }
    }

    // 模板实现, 所有类型stl容器都可用, 遍历容器内容, 转到string中
    template <typename T>
    inline static void PushBackToVec(
        std::stringstream &ss,
        std::vector<std::string> &vec,
        T &value)
    {
        ss.str("");
        ss << value;
        vec.push_back(ss.str());
    }

    template <typename T, template <class, class...> class C, class... Args>
    inline static std::string to_json_array(const C<T, Args...> &contain)
    {
        if (contain.empty())
        {
            return "[]";
        }

        std::stringstream os;
        os << "[" << contain[0];
        for (int idx = 1, contain_size = contain.size(); idx < contain_size; idx++)
        {
            os << ",";
            os << contain[idx];
        }
        os << "]";
        return os.str();
    }

    // google::protobuf::RepeatedField<>
    template <typename T>
    inline static std::string to_json_array(const T &contain)
    {
        if (contain.empty())
        {
            return "[]";
        }

        std::stringstream os;
        os << "[" << contain[0];
        for (int idx = 1, contain_size = contain.size(); idx < contain_size; idx++)
        {
            os << ",";
            os << contain[idx];
        }
        os << "]";
        return os.str();
    }

    // Set求差集, 出现于a但不出现于b的元素, a - b
    template <typename T>
    inline std::vector<T> SetDifference(
        const std::set<T> &a,
        const std::set<T> &b)
    {
        std::vector<int> res(std::max(a.size(), b.size()));
        auto iter = std::set_difference(a.begin(), a.end(),
                                        b.begin(), b.end(), res.begin());
        res.resize(iter - res.begin());
        return std::move(res);
    }

    inline void SplitString(
        const std::string &sourceStr,
        char c,
        std::vector<std::string> &vecRetStr)
    {
        vecRetStr.clear();
        size_t i = 0;
        size_t j = sourceStr.find(c);
        while (j != std::string::npos)
        {
            vecRetStr.push_back(sourceStr.substr(i, j - i));
            i = ++j;
            j = sourceStr.find(c, j);
        }
        // 当while函数退出时, 把最后的字符串放入
        vecRetStr.push_back(sourceStr.substr(i, sourceStr.length()));
    }

    template <typename... Args>
    inline std::string str_format(const std::string &format, Args... args)
    {
        auto size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
        char bytes[size] = {0};
        std::snprintf(bytes, size, format.c_str(), args...);
        return std::string(bytes);
    }

    inline std::string &trim(std::string &str, char c = ' ')
    {
        if (!str.empty())
        {
            str.erase(0, str.find_first_not_of(c));
            str.erase(str.find_last_not_of(c) + 1);
        }
        return str;
    }

    inline std::string &tolower(std::string &str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return str;
    }

    inline std::string tolower(const std::string &str)
    {
        std::string ret = str;
        return tolower(ret);
    }

    template <typename Key, typename Value>
    inline const std::vector<Key> GetHashMapKey(
        const std::unordered_map<Key, Value> &hash_map) noexcept
    {
        std::vector<Key> vec_key;
        vec_key.reserve(hash_map.size());
        for (const auto &item : hash_map)
        {
            vec_key.emplace_back(item.first);
        }
        return vec_key;
    }

    inline bool IsDigit(const std::string &sInput)
    {
        std::string::const_iterator iter = sInput.begin();

        if (sInput.empty())
        {
            return false;
        }

        while (iter != sInput.end())
        {
            if (!isdigit(*iter))
            {
                return false;
            }
            ++iter;
        }
        return true;
    }

} // namespace Common