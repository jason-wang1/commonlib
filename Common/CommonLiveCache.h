#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>

#include "SafeQueue.h"

// 公共实时缓存, 支持HashMap实时缓存方案
class LiveCacheParam;

// 缓存接口
class LiveCacheInterface
{
public:
    virtual int32_t Init(const LiveCacheParam &liveCacheParam) = 0;
    virtual int32_t Timer() = 0;
    virtual int32_t Handle() = 0;
    virtual int32_t ShutDown() = 0;
};

// 缓存参数
class LiveCacheParam
{
public:
    void SetLiveCacheName(const std::string &name) noexcept
    {
        m_name = name;
    }

    void SetTimerInterval(const long timerInterval) noexcept
    {
        m_timerInterval = timerInterval;
    }

    void SetHandleInterval(const long handleInterval) noexcept
    {
        m_handleInterval = handleInterval;
    }

    void SetRefreshTime(const long refreshTime) noexcept
    {
        m_refreshTime = refreshTime;
    }

    void SetRefreshIncrTime(const long refreshIncrTime) noexcept
    {
        m_refreshIncrTime = refreshIncrTime;
    }

    void SetHandleQueueTime(const long handleQueueTime) noexcept
    {
        m_handleQueueTime = handleQueueTime;
    }

    void SetDataValidTime(const long dataValidTime) noexcept
    {
        m_DataValidTime = dataValidTime;
    }

    void SetCacheMaxSize(const long cacheMaxSize) noexcept
    {
        m_CacheMaxSize = cacheMaxSize;
    }

    std::string GetLiveCacheName() const noexcept { return m_name; }
    long GetTimerInterval() const noexcept { return m_timerInterval; }
    long GetHandleInterval() const noexcept { return m_handleInterval; }
    long GetRefreshTime() const noexcept { return m_refreshTime; }
    long GetRefreshIncrTime() const noexcept { return m_refreshIncrTime; }
    long GetHandleQueueTime() const noexcept { return m_handleQueueTime; }
    long GetDataValidTime() const noexcept { return m_DataValidTime; }
    long GetCacheMaxSize() const noexcept { return m_CacheMaxSize; }

public:
    void SetStrParam(const std::string &key, const std::string &value) noexcept
    {
        m_StrParam[key] = value;
    }

    void SetLongParam(const std::string &key, const long value) noexcept
    {
        m_LongParam[key] = value;
    }

    std::string GetStrParam(const std::string &key) const noexcept
    {
        if (const auto it = m_StrParam.find(key);
            it != m_StrParam.end())
        {
            return it->second;
        }
        return "";
    }

    long GetLongParam(const std::string &key) const noexcept
    {
        if (const auto it = m_LongParam.find(key);
            it != m_LongParam.end())
        {
            return it->second;
        }
        return 0;
    }

private:
    std::string m_name;
    long m_timerInterval = 0;
    long m_handleInterval = 0;
    long m_refreshTime = 0;
    long m_refreshIncrTime = 0;
    long m_handleQueueTime = 0;
    long m_DataValidTime = 0;
    long m_CacheMaxSize = 0;

    std::unordered_map<std::string, std::string> m_StrParam;
    std::unordered_map<std::string, long> m_LongParam;
};

template <typename Key, typename Value>
class HashLiveCache
{
public:
    struct LiveDataInfo
    {
        long lastUpdateTime = 0;
        Value liveData;
    };

    using LiveDataInfoUMap = std::unordered_map<Key, LiveDataInfo>;

    void SetDatavalidTime(const long dataValidTime) noexcept
    {
        m_DataValidTime = dataValidTime;
    }

    long GetDataValidTime() const noexcept
    {
        return m_DataValidTime;
    }

public:
    bool GetLiveCacheData(const Key key, const long nowTime, Value &liveCacheData) const noexcept
    {
        // P.s> 这里之所以用find标识, 而不是 liveCacheData.empty(), 因为数据可能就是空.
        bool find = false;
        {
            const auto &buf = m_dBufLiveCache[m_dataIdx];
            if (auto it = buf.find(key); it != buf.end())
            {
                if (nowTime < (it->second.lastUpdateTime + m_DataValidTime))
                {
                    liveCacheData = it->second.liveData;
                    find = true;
                }
            }
        }

        // 如果为空, 说明双Buffer里面没有, 在读写锁区域再找一次
        if (!find)
        {
            std::shared_lock<std::shared_mutex> sl(m_rwLock);
            if (auto it = m_rwLiveCache.find(key); it != m_rwLiveCache.end())
            {
                if (nowTime < (it->second.lastUpdateTime + m_DataValidTime))
                {
                    liveCacheData = it->second.liveData;
                    find = true;
                }
            }
        }

        return find;
    }

    // 将数据copy到读写锁缓存中
    void AddRWBufLiveCacheData(const std::unordered_map<Key, LiveDataInfo> &liveCacheData) noexcept
    {
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        for (const auto &item : liveCacheData)
        {
            auto &liveDataInfo = m_rwLiveCache[item.first];
            liveDataInfo.lastUpdateTime = item.second.lastUpdateTime;
            liveDataInfo.liveData = item.second.liveData;
        }
    }

    // 将数据copy到双Buffer缓存中
    void AddDBufLiveCacheData(const std::unordered_map<Key, LiveDataInfo> &liveCacheData) noexcept
    {
        // 初始化的时候调用一次, 后续都由工作线程调用, 直接写入双Buffer
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        auto &newBuffer = m_dBufLiveCache[newDataIdx];
        auto &curBuffer = m_dBufLiveCache[curDataIdx];

        newBuffer = liveCacheData;
        newBuffer.insert(curBuffer.begin(), curBuffer.end());

        m_dataIdx = newDataIdx;
    }

    // 将数据move到双Buffer缓存中
    void AddDBufLiveCacheData(std::unordered_map<Key, LiveDataInfo> &&liveCacheData) noexcept
    {
        // 初始化的时候调用一次, 后续都由工作线程调用, 直接写入双Buffer
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        auto &newBuffer = m_dBufLiveCache[newDataIdx];
        auto &curBuffer = m_dBufLiveCache[curDataIdx];

        newBuffer = std::move(liveCacheData);
        newBuffer.insert(curBuffer.begin(), curBuffer.end());

        m_dataIdx = newDataIdx;
        liveCacheData.clear();
    }

    // 设置新的缓存
    void SetDBufLiveCacheData(const long nowTime, const std::unordered_map<Key, Value> &liveCacheData) noexcept
    {
        int newDataIdx = (m_dataIdx + 1) % 2;

        auto &newBuffer = m_dBufLiveCache[newDataIdx];
        for (const auto &item : liveCacheData)
        {
            auto &liveDataInfo = newBuffer[item.first];
            liveDataInfo.lastUpdateTime = nowTime;
            liveDataInfo.liveData = item.second;
        }

        m_dataIdx = newDataIdx;
    }

    // 清理缓存缓存无效数据
    long ClearDBufLiveCacheData(const long nowTime) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (m_dataIdx + 1) % 2;

        auto &newBuffer = m_dBufLiveCache[newDataIdx];
        auto &curBuffer = m_dBufLiveCache[curDataIdx];

        long clear_num = 0;
        newBuffer.clear();
        for (auto &item : curBuffer)
        {
            if (nowTime >= (item.second.lastUpdateTime + m_DataValidTime))
            {
                clear_num--;
                continue;
            }

            newBuffer.insert(std::make_pair(item.first, item.second));
        }
        m_dataIdx = newDataIdx;

        return clear_num;
    }

    // 刷新缓存, 将读写锁数据刷新到双缓冲中
    void RefreshLiveCache() noexcept
    {
        // 将读写锁内数据移动到双缓冲, 清空读写锁内数据
        {
            std::unique_lock<std::shared_mutex> ul(m_rwLock);
            AddDBufLiveCacheData(std::move(m_rwLiveCache));
            m_rwLiveCache.clear();
        }
    }

    // 获取双缓冲Buffer内key数量
    const int GetDBufSize() const noexcept
    {
        int curDataIdx = m_dataIdx;
        return m_dBufLiveCache[curDataIdx].size();
    }

    // 获取读写锁内key数量
    const int GetRWBufSize() const noexcept
    {
        std::shared_lock<std::shared_mutex> sl(m_rwLock);
        return m_rwLiveCache.size();
    }

    void PushLiveDataQueue(const long nowTime, const std::unordered_map<Key, Value> &liveCacheData)
    {
        LiveDataInfoUMap newLiveData;
        for (const auto &item : liveCacheData)
        {
            auto &liveDataInfo = newLiveData[item.first];
            liveDataInfo.lastUpdateTime = nowTime;
            liveDataInfo.liveData = item.second;
        }

        m_LiveDataQueue.push_back(newLiveData);
    }

    // 处理实时数据队列，将实时数据队列数据刷新到读写锁中
    long HandleLiveDataQueue(const long batchCount) noexcept
    {
        long handle_num = 0;
        std::vector<LiveDataInfoUMap> vecLiveDataList;
        if (m_LiveDataQueue.pop_front(vecLiveDataList, batchCount, 0, false))
        {
            LiveDataInfoUMap newLiveData;
            long cmdSize = vecLiveDataList.size();
            for (long idx = cmdSize - 1; idx >= 0; idx--)
            {
                handle_num++;
                auto &item = vecLiveDataList[idx];
                newLiveData.insert(item.begin(), item.end());
            }

            if (newLiveData.size() > 0)
            {
                AddRWBufLiveCacheData(newLiveData);
            }
        }

        return handle_num;
    }

public:
    // 双缓冲
    std::atomic<int> m_dataIdx = 0;
    std::unordered_map<Key, LiveDataInfo> m_dBufLiveCache[2];

    // 读写锁
    mutable std::shared_mutex m_rwLock;
    std::unordered_map<Key, LiveDataInfo> m_rwLiveCache;

    SafeQueue<LiveDataInfoUMap> m_LiveDataQueue;

    long m_DataValidTime = 0;
};

template <typename Value>
class SingleLiveCache
{
public:
    struct LiveDataInfo
    {
        long lastUpdateTime = 0;
        Value liveData;
    };

    void SetDatavalidTime(const long dataValidTime) noexcept
    {
        m_DataValidTime = dataValidTime;
    }

    long GetDataValidTime() const noexcept
    {
        return m_DataValidTime;
    }

public:
    inline bool GetCacheData(const long nowTime, Value &cacheData) const noexcept
    {
        const auto &buf = m_cacheData[m_dataIdx];
        if (nowTime < (buf.lastUpdateTime + m_DataValidTime))
        {
            cacheData = buf.liveData;
            return true;
        }

        return false;
    }

    inline void SetCacheData(const long nowTime, const Value &cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        auto &newBuffer = m_cacheData[newDataIdx];
        newBuffer.liveData = cacheData;
        newBuffer.lastUpdateTime = nowTime;

        m_dataIdx = newDataIdx;
    }

    inline void SetCacheData(const long nowTime, Value &&cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        auto &newBuffer = m_cacheData[newDataIdx];
        newBuffer.liveData = std::move(cacheData);
        newBuffer.lastUpdateTime = nowTime;

        m_dataIdx = newDataIdx;
    }

private:
    // 双缓冲
    std::atomic<int> m_dataIdx = 0;
    LiveDataInfo m_cacheData[2];

    long m_DataValidTime = 0;
};
