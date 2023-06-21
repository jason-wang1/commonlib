#pragma once
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

class TDRedis;
class TDRedisConnList;

// TDRedisPtr 通过智能指针 unique_ptr 实现资源管理
// 当析构时调用 TDRedisDeleter 完成资源归还.
struct TDRedisDeleter
{
    void operator()(TDRedis *) const;
};
using TDRedisPtr = std::unique_ptr<TDRedis, TDRedisDeleter>;

class TDRedisConnPool final
{
    // 声明仿函数友元
    friend struct TDRedisDeleter;

public:
    // 不依赖其他库, 实现单例模式
    using Ptr = std::shared_ptr<TDRedisConnPool>;
    static Ptr GetInstance();

    // 添加一个连接池
    bool AddConnPool(
        const std::string &name,
        const std::string &ip,
        const int port,
        const bool crypto,
        const std::string &password,
        const int index,
        const int initPoolSize,
        const int timeout = 1000);

    // 获取连接
    TDRedisPtr GetConnect(const std::string &name) const;

    // 获取连接数量
    int GetConnPoolSize(const std::string &name) const;

    // 停用所有连接池
    // 停用后 不再支持GetConnect.
    // 停用后 连接资源自动释放时会直接释放连接.
    void ShutDown() const;

    virtual ~TDRedisConnPool() { ShutDown(); };
    TDRedisConnPool(TDRedisConnPool &) = delete;
    TDRedisConnPool &operator=(const TDRedisConnPool &) = delete;

protected:
    // 返还连接
    // tkxiong 2021.07.31 资源自动管理之后就不需要手动返还了, 故方法不再对外暴露.
    // tkxiong 2023.05.16 函数内根据连接情况决定是否需要reset, 故只保留ReturnConnect
    bool ReturnConnect(TDRedis *&ret) const;

private:
    TDRedisConnPool() {}

    // 读写锁, 只防止map Rehash, 不保证map内数据修改
    mutable std::shared_mutex m_Name2ConnListLock;
    std::unordered_map<std::string, std::shared_ptr<TDRedisConnList>> m_Name2ConnList;

    static Ptr m_instance_ptr; // 单例
    static std::mutex m_mutex; // 互斥锁
};
