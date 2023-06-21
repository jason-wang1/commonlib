
#pragma once
#include <string>
#include <sys/time.h>
namespace Common
{
    // 获取当前时间戳
    inline double get_ms_time() noexcept
    {
        struct timeval timev;
        if (gettimeofday(&timev, NULL) == 0)
        {
            return (double)timev.tv_sec * (double)1000 +
                   (double)timev.tv_usec * .001;
        }
        return 0.0;
    }

    inline long get_ns_timestamp() noexcept
    {
        constexpr long NS_PER_SECOND = 1000000000;
        constexpr long NS_PER_USECOND = 1000;
        struct timeval timev;
        if (gettimeofday(&timev, NULL) == 0)
        {
            return timev.tv_sec * NS_PER_SECOND + timev.tv_usec * NS_PER_USECOND;
        }
        return 0;
    }

    // 获取当前时间戳
    inline long get_ms_timestamp() noexcept
    {
        struct timeval timev;
        if (gettimeofday(&timev, NULL) == 0)
        {
            return timev.tv_sec * (long)1000 + timev.tv_usec / 1000;
        }
        return 0;
    }

    // 获取当前秒级时间戳
    inline long get_timestamp() noexcept
    {
        return get_ms_timestamp() / 1000;
    }

    // 获取当天0点的秒级时间戳, 以服务器当前时区为准
    inline long get_today_timestamp() noexcept
    {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        tm->tm_hour = 0;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        return mktime(tm);
    }

    // 时间戳转日期 (单位s)
    inline std::string timestamp2date(
        const long timestamp,
        const std::string date_format = "%Y-%m-%d %H:%M:%S") noexcept
    {
        struct tm tt;
        localtime_r(&timestamp, &tt);
        char sTime[32] = {0};
        strftime(sTime, sizeof(sTime), date_format.c_str(), &tt);
        return std::string(sTime);
    }

    // 时间戳转日期 (单位s)
    inline std::string timestamp2date_utc(
        const long timestamp,
        const std::string date_format = "%Y-%m-%d %H:%M:%S") noexcept
    {
        struct tm tt;
        gmtime_r(&timestamp, &tt);
        char sTime[32] = {0};
        strftime(sTime, sizeof(sTime), date_format.c_str(), &tt);
        return std::string(sTime);
    }

    // 时间转日期 "时区 年-月-日 时:分:秒.微秒"
    inline std::string ms_time2date(const double ms_time) noexcept
    {
        return timestamp2date(long(ms_time) / 1000, "%z %Y-%m-%d %H:%M:%S")
            .append(".")
            .append(std::to_string(long(ms_time * 1000) % 1000000));
    }
}