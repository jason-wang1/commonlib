#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>

// 公共缓存, 支持Hash, Vector, Single三种缓存方案
// 如果需要可以按规则拓展...Set这样的
// 目前主要用于RedisProtoData/LocalCache
// 其他需求可以联系 tkxiong
// 还在修改中, 接口大概率不会变.

class CacheParam;

// 缓存接口
class CacheInterface
{
public:
    virtual int32_t Init(const CacheParam &cacheParam) = 0;
    virtual int32_t Refresh() = 0;
    virtual int32_t RefreshIncr() = 0;
    virtual int32_t ShutDown() = 0;
};

// 缓存参数
class CacheParam
{
public:
    void SetCacheName(const std::string &name) noexcept { m_name = name; }
    void SetRefreshTime(const long refreshTime) noexcept { m_refreshTime = refreshTime; }
    void SetRefreshIncrTime(const long refreshIncrTime) noexcept { m_refreshIncrTime = refreshIncrTime; }

    std::string GetCacheName() const noexcept { return m_name; }
    long GetRefreshTime() const noexcept { return m_refreshTime; }
    long GetRefreshIncrTime() const noexcept { return m_refreshIncrTime; }

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
    long m_refreshTime = 0;
    long m_refreshIncrTime = 0;

    std::unordered_map<std::string, std::string> m_StrParam;
    std::unordered_map<std::string, long> m_LongParam;
};

// 缓存基类
class CacheBase
{
public:
    CacheBase() {}
    virtual ~CacheBase() {}

public:
    // 缓存数据时间戳, 每次修改缓存时修改
    long GetCacheTimeStamp() const noexcept { return m_timeStamp; }
    void SetCacheTimeStamp(long time) noexcept { m_timeStamp = time; }

private:
    long m_timeStamp = 0; // 当前缓存 unix时间戳
};

template <typename Key, typename Value>
class HashCache : public CacheBase
{
public:
    bool GetDBufCacheData(const Key &key, Value &cacheData) const noexcept
    {
        const auto spDBuf = m_dBufCache[m_dataIdx];
        if (spDBuf != nullptr)
        {
            const auto it = spDBuf->find(key);
            if (it != spDBuf->end())
            {
                cacheData = it->second;
                return true;
            }
        }
        return false;
    }

    bool GetRWCacheData(const Key &key, Value &cacheData) const noexcept
    {
        // 20220912 tkxiong 尝试获取锁, 获取失败时不查找, 以免引起阻塞
        // owns_lock() Checks whether *this owns a locked mutex or not.
        std::shared_lock<std::shared_mutex> sl(m_rwLock, std::try_to_lock);
        if (sl.owns_lock())
        {
            if (const auto it = m_rwCache.find(key); it != m_rwCache.end())
            {
                cacheData = it->second;
                return true;
            }
        }
        return false;
    }

    bool GetCacheData(const Key &key, Value &cacheData) const noexcept
    {
        if (GetDBufCacheData(key, cacheData))
        {
            return true;
        }
        return GetRWCacheData(key, cacheData);
    }

    // 将数据copy到读写锁缓存中
    void AddRWBufCacheData(const Key &key, const Value &value) noexcept
    {
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        m_rwCache[key] = value;
    }

    // 将数据copy到读写锁缓存中
    void AddRWBufCacheData(const Key &key, Value &&value) noexcept
    {
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        m_rwCache[key] = std::move(value);
    }

    // 将数据copy到读写锁缓存中
    void AddRWBufCacheData(const std::unordered_map<Key, Value> &cacheData) noexcept
    {
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        for (const auto &item : cacheData)
        {
            m_rwCache[item.first] = item.second;
        }
    }

    // 将数据move到读写锁缓存中
    void AddRWBufCacheData(std::unordered_map<Key, Value> &&cacheData) noexcept
    {
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        if (m_rwCache.size() < cacheData.size())
        {
            cacheData.insert(m_rwCache.begin(), m_rwCache.end());
            m_rwCache = std::move(cacheData);
            cacheData.clear();
        }
        else
        {
            // 20220922 tkxiong 我认为这里还存在性能问题, 对应优化场景如下:
            // rw_cache内有100条数据, 新cache有30条数据, 它们的重复数据是3条
            // 此时将30条数据写入rw_cache覆盖即可
            for (const auto &item : cacheData)
            {
                m_rwCache[item.first] = item.second;
            }
        }
    }

    void ClearRWBuf() noexcept
    {
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        m_rwCache.clear();
    }

    // 将数据copy到双Buffer缓存中
    void AddDBufCacheData(const std::unordered_map<Key, Value> &cacheData) noexcept
    {
        std::lock_guard<std::mutex> ul(m_updateLock);

        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        // 直接生成新的存储空间, 旧空间引用计数归零后释放
        m_dBufCache[newDataIdx] = std::make_shared<std::unordered_map<Key, Value>>();
        auto spNewBuf = m_dBufCache[newDataIdx];
        spNewBuf->insert(cacheData.begin(), cacheData.end());

        const auto spCurBuf = m_dBufCache[curDataIdx];
        if (spCurBuf != nullptr)
        {
            spNewBuf->insert(spCurBuf->begin(), spCurBuf->end());
        }

        m_dataIdx = newDataIdx;
        RefreshContainKey();
    }

    // 将数据move到双Buffer缓存中
    void AddDBufCacheData(std::unordered_map<Key, Value> &&cacheData) noexcept
    {
        std::lock_guard<std::mutex> ul(m_updateLock);

        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        // 直接生成新的存储空间, 旧空间引用计数归零后释放
        m_dBufCache[newDataIdx] = std::make_shared<std::unordered_map<Key, Value>>();
        auto spNewBuf = m_dBufCache[newDataIdx];
        auto &newBuf = *spNewBuf.get();
        newBuf = std::move(cacheData);
        cacheData.clear();

        const auto spCurBuf = m_dBufCache[curDataIdx];
        if (spCurBuf != nullptr)
        {
            spNewBuf->insert(spCurBuf->begin(), spCurBuf->end());
        }

        m_dataIdx = newDataIdx;
        RefreshContainKey();
    }

    // 设置新的缓存
    void SetDBufCacheData(std::unordered_map<Key, Value> &&cacheData) noexcept
    {
        std::lock_guard<std::mutex> ul(m_updateLock);
        int newDataIdx = (m_dataIdx + 1) % 2;

        // 直接生成新的存储空间, 旧空间引用计数归零后释放
        m_dBufCache[newDataIdx] = std::make_shared<std::unordered_map<Key, Value>>();
        auto spNewBuf = m_dBufCache[newDataIdx];
        auto &newBuf = *spNewBuf.get();
        newBuf = std::move(cacheData);
        cacheData.clear();

        m_dataIdx = newDataIdx;
        RefreshContainKey();
    }

    // 刷新缓存, 将读写锁数据刷新到双缓冲中
    void RefreshCache() noexcept
    {
        // 将读写锁内数据移动到双缓冲, 清空读写锁内数据
        std::unique_lock<std::shared_mutex> ul(m_rwLock);
        AddDBufCacheData(std::move(m_rwCache));
    }

    // 获取双缓冲Buffer内key数量
    const int GetDBufSize() const noexcept
    {
        const int curDataIdx = m_dataIdx;
        const auto spCurDbuf = m_dBufCache[curDataIdx];
        if (spCurDbuf != nullptr)
        {
            return spCurDbuf->size();
        }
        return 0;
    }

    // 获取读写锁内key数量
    const int GetRWBufSize() const noexcept
    {
        std::shared_lock<std::shared_mutex> sl(m_rwLock);
        return m_rwCache.size();
    }

    // 获取当前双缓存内所有的key
    const std::unordered_set<Key> GetContainKey() const noexcept
    {
        return m_DbufContainKey;
    }

    // 获取当前双缓存内所有key的数量
    const int GetContainSize() const noexcept
    {
        return m_DbufContainKey.size();
    }

protected:
    // 将当前双缓冲内key同步到ContainKey中
    void RefreshContainKey() noexcept
    {
        m_DbufContainKey.clear();

        const auto spCurBuf = m_dBufCache[m_dataIdx];
        if (spCurBuf != nullptr)
        {
            auto &buf = *spCurBuf.get();
            for (const auto &item : buf)
            {
                m_DbufContainKey.insert(item.first);
            }
        }
    }

public:
    // 双缓冲
    std::mutex m_updateLock;
    std::atomic<int> m_dataIdx = 0;
    std::shared_ptr<std::unordered_map<Key, Value>> m_dBufCache[2];
    std::unordered_set<Key> m_DbufContainKey; // 记录当前双缓存内的key

    // 读写锁
    mutable std::shared_mutex m_rwLock;
    std::unordered_map<Key, Value> m_rwCache;
};

template <typename Value>
class VectorCache : public CacheBase
{
public:
    bool GetCacheData(int index, Value &cacheData) const noexcept
    {
        int curDataIdx = m_dataIdx;
        auto &vecCacheData = m_vecCacheData[curDataIdx];
        if (vecCacheData.empty())
        {
            return false;
        }

        int size = static_cast<int>(vecCacheData.size());
        if (index >= size || index < 0)
        {
            return false;
        }

        cacheData = vecCacheData[index];
        return true;
    }

    inline void SetCacheData(const std::vector<Value> &cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;
        m_vecCacheData[newDataIdx] = cacheData;
        m_dataIdx = newDataIdx;
    }

    inline void SetCacheData(std::vector<Value> &&cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;
        m_vecCacheData[newDataIdx] = std::move(cacheData);
        cacheData.clear();
        m_dataIdx = newDataIdx;
    }

    // 添加增量数据到尾部
    void AppendCacheData(const std::vector<Value> &cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        const auto &vecCurCache = m_vecCacheData[curDataIdx];
        auto &vecNewCache = m_vecCacheData[newDataIdx];

        // 预申请合适大小的空间
        vecNewCache.reserve(vecCurCache.size() + cacheData.size());

        // 先将当前数据覆盖新数据, 作为Base
        vecNewCache = vecCurCache;

        // 再将新增数据写入
        for (const auto &item : cacheData)
        {
            vecNewCache.emplace_back(item);
        }

        m_dataIdx = newDataIdx;
    }

    // 添加数据到尾部
    void AppendCacheData(std::vector<Value> &&cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;

        const auto &vecCurCache = m_vecCacheData[curDataIdx];
        auto &vecNewCache = m_vecCacheData[newDataIdx];

        // 预申请合适大小的空间
        vecNewCache.reserve(vecCurCache.size() + cacheData.size());

        // 先将当前数据覆盖新数据, 作为Base
        vecNewCache = vecCurCache;

        // 再将新增数据写入
        for (auto &item : cacheData)
        {
            vecNewCache.emplace_back(std::move(item));
        }
        cacheData.clear();

        m_dataIdx = newDataIdx;
    }

    // 获取双缓冲Buffer内key数量
    const int GetCacheSize() const noexcept
    {
        int curDataIdx = m_dataIdx;
        return m_vecCacheData[curDataIdx].size();
    }

private:
    // 双缓冲
    std::atomic<int> m_dataIdx = 0;
    std::vector<Value> m_vecCacheData[2];
};

template <typename Value>
class SingleCache : public CacheBase
{
public:
    inline bool GetCacheData(Value &cacheData) const noexcept
    {
        cacheData = m_cacheData[m_dataIdx];
        return true;
    }

    inline void SetCacheData(const Value &cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;
        m_cacheData[newDataIdx] = cacheData;
        m_dataIdx = newDataIdx;
    }

    inline void SetCacheData(Value &&cacheData) noexcept
    {
        int curDataIdx = m_dataIdx;
        int newDataIdx = (curDataIdx + 1) % 2;
        m_cacheData[newDataIdx] = std::move(cacheData);
        m_dataIdx = newDataIdx;
    }

private:
    // 双缓冲
    std::atomic<int> m_dataIdx = 0;
    Value m_cacheData[2];
};
