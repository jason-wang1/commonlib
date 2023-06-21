#pragma once
#include <string>
#include <unistd.h> // geteuid

namespace Shell
{
    // 判断当前用户为root用户
    inline bool is_root() noexcept
    {
        return (geteuid() == 0);
    }

    // 执行shell命令
    // [成功] 返回true, result = 返回内容
    // [失败] 返回false, result = 错误信息
    inline bool exec(const std::string &cmd, std::string &result) noexcept
    {
        result.clear();
        FILE *pFile = NULL;
        if (NULL == (pFile = popen(cmd.c_str(), "r")))
        {
            result = "execute shell command error";
            return false;
        }

        int bufferSize = 10240; // 10KB应该是非常充足了
        char *buffer = new char[bufferSize];
        while (NULL != fgets(buffer, bufferSize, pFile))
        {
            result += buffer;
        }
        delete[] buffer;
        pclose(pFile);
        return true;
    }
} // namespace Shell