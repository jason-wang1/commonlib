#pragma once
#include <string>
#include <vector>
#include <sstream>

struct redisContext;

// TDRedis不保证线程安全, 加锁也不行; 单个TDRedis对象应在单线程内使用.
// 参考链接: https://support.huaweicloud.com/productdesc-dcs/ChooseCache.html
// 命令标准实现: https://redis.io/commands
class TDRedis final
{
    friend class TDRedisConnList;
    friend class TDRedisConnPool;

public:
    // redis 错误码
    enum Error
    {
        OK = 0,
        Redis_IO = 1,       // Redis Error in read or write
        Redis_Other = 2,    // Redis Error Everything else...
        Redis_EOF = 3,      // Redis Error End of file
        Redis_Protocol = 4, // Redis Error Protocol error
        Redis_OOM = 5,      // Redis Error Out of memory
        Redis_TimeOut = 6,  // Redis Error Timed Out
        NormalError = 100,  // redis常规错误, 语法错误, 用法错误等等
        NetError,           // 网络请求错误
        ParamsError,        // 参数错误, 参数为空等等...
        ReplyError,         // 返回类型错误
        AUTHError,          // 密码验证失败, ERR invalid password
        PINGError,          // Ping返回出错
        SELECTError,        // SELECT返回出错
    };

public:
    // 拿走最后一次访问的错误信息, 只能拿取一次, 重连时所有错误信息会被清理
    const std::string GetLastError() noexcept;

    // 获取上一次请求的错误码, 重连时清理
    Error GetErrorCode() const noexcept;

    // 获取当前连接Pipe剩余请求数量
    const int GetRequestCount() const noexcept;

    // 获取当前连接名称
    const std::string GetName() const noexcept;

    Error PING();

    // 错误调用
    // 不会发送网络请求, 但是模拟一次错误的请求结果
    Error ErrorTest();

    // 华为云Redis集群禁用SAVE和BGSAVE
    // Error BGSAVE();
    // Error SAVE();

    // 设置命令超时时间(请求失败重连后会重置该时间)
    Error SetCommandTimeout(const int timeout /*ms*/);

public:
    // Key 参考: https://www.runoob.com/redis/redis-keys.html

    Error UNLINK(const std::string &key, long &retCount);
    Error UNLINK(const std::vector<std::string> &keys, long &retCount);
    Error EXISTS(const std::string &key, bool &retExist);
    Error EXISTS(const std::vector<std::string> &keys, bool &retExist); // 任意key存在即返回1.

    // count 参考值为10 ~ 1000
    Error SCAN(const long cursor, const std::string &pattern, const long count,
               long &retCursor, std::vector<std::string> &retKeys);

    // DEL命令, 若Redis版本大于4.0, 使用UNLINK代替
    //      Hash类型, 用 HSCAN + HDEL.
    //      Set 类型, 用 SSCAN + SREM.
    //      ZSet类型, 用 ZSCAN + ZREM.
    //      List类型, 建议半夜(不影响业务服务的情况下)用ltrim.
    // Error DEL(const std::string &key, long &count);
    // Error DEL(const std::vector<std::string> &keys, long &count);

public:
    // String字符串, 参考: https://www.runoob.com/redis/redis-strings.html

    Error SET(const std::string &key, const std::string &value);

    /**
     * @brief SET命令 增加 PX NX 参数
     * @param milliseconds 过期时间, 传0标识不使用PX参数
     * @param retVal 返回值: 1 成功; 0 失败;
     **/
    Error SETNX(const std::string &key, const std::string &value, const long milliseconds, long &retVal);

    Error MSET(const std::vector<std::string> &keys, const std::vector<std::string> &values);

    Error GET(const std::string &key, std::string &retVal);
    Error PipeGET(const std::string &key);
    Error PipeGETRet(std::string &retVal);

    Error MGET(const std::vector<std::string> &keys, std::vector<std::string> &retVals);
    Error PipeMGET(const std::vector<std::string> &keys);
    Error PipeMGETRet(std::vector<std::string> &retVal);

    Error GETRANGE(const std::string &key, const long start, const long end, std::string &retVal);
    Error GETSET(const std::string &key, const std::string &value, std::string &retVal);

    Error STRLEN(const std::string &key, long &retLength);
    Error APPEND(const std::string &key, const std::string &value, long &retLength);

public:
    // Hash哈希, 参考: https://www.runoob.com/redis/redis-hashes.html
    Error HKEYS(const std::string &key, std::vector<std::string> &retFields);

    Error HSET(const std::string &key, const std::string &field, const std::string &value);
    Error PipeHSET(const std::string &key, const std::string &field, const std::string &value);
    Error PipeHSETRet();
    Error HMSET(const std::string &key, const std::vector<std::string> &fields, const std::vector<std::string> &values);

    Error HGET(const std::string &key, const std::string &field, std::string &retVal);
    Error PipeHGET(const std::string &key, const std::string &field);
    Error PipeHGETRet(std::string &retVal);
    Error HMGET(const std::string &key, const std::vector<std::string> &fields, std::vector<std::string> &retVals);
    Error PipeHMGET(const std::string &key, const std::vector<std::string> &fields);
    Error PipeHMGETRet(std::vector<std::string> &retVals);

    // HSCAN, count 单次获取数量参考值为10 ~ 1000
    Error HSCAN(const std::string &key, const long cursor, const std::string &pattern, const long count,
                long &retCursor, std::vector<std::string> &retKeys, std::vector<std::string> &retVals);
    Error HEXISTS(const std::string &key, const std::string &field, bool &retExist);
    Error HLEN(const std::string &key, long &retCount);
    Error PipeHLEN(const std::string &key);
    Error PipeHLENRet(long &value);
    Error HDEL(const std::string &key, const std::string &field, long &retCount);
    // HDEL, 建议单次删除数量不超过100, 如果超过最好多次调用.
    Error HDEL(const std::string &key, const std::vector<std::string> &fields, long &retCount);
    Error PipeHDEL(const std::string &key, const std::string &field);
    Error PipeHDELRet(long &retCount);

    // HINCRBY 可对整数做加法
    Error HINCRBY(const std::string &key, const std::string &field, const long increment, long &retCount);
    Error PipeHINCRBY(const std::string &key, const std::string &field, const long increment);
    Error PipeHINCRBYRet(long &retCount);

    // HINCRBYFLOAT 可对浮点数做加法
    Error HINCRBYFLOAT(const std::string &key, const std::string &field, const double increment, double &retCount);

    // P.s> 对于大key, 不可用下列方法, 尽量用 HSCAN 方法获取
    // Error HGETALL(const std::string &key, std::vector<std::string> &retFields, std::vector<std::string> &retVals);
    // Error HKEYS(const std::string &key, std::vector<std::string> &retFields);
    // Error HVALS(const std::string &key, std::vector<std::string> &retVals);

public:
    // List列表, 命令参考: https://www.runoob.com/redis/redis-lists.html

    Error LPUSH(const std::string &key, const std::string &value);
    Error LPUSH(const std::string &key, const std::vector<std::string> &values);
    Error LPOP(const std::string &key, std::string &retVal);
    Error RPUSH(const std::string &key, const std::string &value);
    Error RPUSH(const std::string &key, const std::vector<std::string> &values);
    Error RPOP(const std::string &key, std::string &retVal);
    Error PipeRPOP(const std::string &key);
    Error PipeRPOPRet(std::string &retVal);

    Error LLEN(const std::string &key, long &retLen);
    Error LREM(const std::string &key, const long count, const std::string &element);
    Error LRANGE(const std::string &key, const long start, const long stop, std::vector<std::string> &retVals);
    Error LTRIM(const std::string &key, const long start, const long stop);
    Error LSET(const std::string &key, const long index, const std::string &value);
    Error LINDEX(const std::string &key, const long index, std::string &retVal); // 命令时间复杂度为O(N)

public:
    // Set集合, 命令参考: https://www.runoob.com/redis/redis-sets.html

    Error SADD(const std::string &key, const std::string &member);
    Error PipeSADD(const std::string &key, const std::string &member);
    Error PipeSADDRet(long &retValue);
    Error SADD(const std::string &key, const std::vector<std::string> &members);
    Error SISMEMBER(const std::string &key, const std::string &member, long &retIsMember);
    Error PipeSISMEMBER(const std::string &key, const std::string &member);
    Error PipeSISMEMBERRet(long &retIsMember);
    Error SCARD(const std::string &key, long &retCount);
    Error PipeSCARD(const std::string &key);
    Error PipeSCARDRet(long &retCount);
    Error SREM(const std::string &key, const std::string &member);
    Error PipeSREM(const std::string &key, const std::string &member);
    Error PipeSREMRet();
    Error SREM(const std::string &key, const std::vector<std::string> &members);
    // count 参考值为10 ~ 1000
    Error SSCAN(const std::string &key, const long cursor, const std::string &pattern, const long count,
                long &retCursor, std::vector<std::string> &retMembers);

    // 随机返回Set中的N个元素
    Error SRANDMEMBER(const std::string &key, const long count, std::vector<std::string> &retMembers);

    // P.s> 对于大key, 不可用下列方法, 请用 SSCAN 方法获取
    // Error SMEMBERS(const std::string &key, std::vector<std::string> &retMembers);

public:
    // Sorted Set 有序集合, 命令参考: https://www.runoob.com/redis/redis-sorted-sets.html

    // ZADD 添加元素; 两个重载PipeZADDRet的实现是一样的
    Error ZADD(const std::string &key, const double score, const std::string &member, long &retCount);
    Error PipeZADD(const std::string &key, const double score, const std::string &member);
    Error ZADD(const std::string &key, const std::vector<double> &scores, const std::vector<std::string> &members, long &retCount);
    Error PipeZADD(const std::string &key, const std::vector<double> &scores, const std::vector<std::string> &members);
    Error PipeZADDRet(long &retCount);

    // ZINCRBY元素添加增量,返回计算结果
    Error ZINCRBY(const std::string &key, const double increment, const std::string &member, std::string &retString);
    Error PipeZINCRBY(const std::string &key, const double increment, const std::string &member);
    Error PipeZINCRBYRet(std::string &retString);

    Error ZScore(const std::string &key, const std::string &member, double &retScore);
    Error PipeZScore(const std::string &key, const std::string &member);
    Error PipeZScoreRet(double &retScore);

    Error ZCARD(const std::string &key, long &retCount);
    Error PipeZCARD(const std::string &key);
    Error PipeZCARDRet(long &retCount);

    // ZREM 移除元素; 两个重载PipeZREMRet的实现是一样的
    Error ZREM(const std::string &key, const std::string &member, long &retCount);
    Error PipeZREM(const std::string &key, const std::string &member);
    Error ZREM(const std::string &key, const std::vector<std::string> &members, long &retCount);
    Error PipeZREM(const std::string &key, const std::vector<std::string> &members);
    Error PipeZREMRet(long &retCount);

    // 按分数从小到大排序
    Error ZRANK(const std::string &key, const std::string &member, long &retRank);

    Error ZRANGE(const std::string &key, const long start, const long stop, std::vector<std::string> &retMembers);
    Error PipeZRANGE(const std::string &key, const long start, const long stop);
    Error PipeZRANGERet(std::vector<std::string> &retMembers);

    Error ZRANGEWITHSCORES(const std::string &key,
                           const long start,
                           const long stop,
                           std::vector<std::string> &retMembers,
                           std::vector<double> &retScores);

    Error ZRANGEBYSCORE(
        const std::string &key,
        const double min,
        const double max,
        std::vector<std::string> &retMembers);
    Error ZRANGEBYSCORE(
        const std::string &key,
        const double min,
        const double max,
        const long offset,
        const long count,
        std::vector<std::string> &retMembers);

    // 按分数从大到小排序
    Error ZREVRANK(const std::string &key, const std::string &member, long &retRank);

    Error ZREVRANGE(const std::string &key, const long start, const long stop, std::vector<std::string> &retMembers);
    Error PipeZREVRANGE(const std::string &key, const long start, const long stop);
    Error PipeZREVRANGERet(std::vector<std::string> &retMembers);

    Error ZREVRANGEWITHSCORES(const std::string &key,
                              const long start,
                              const long stop,
                              std::vector<std::string> &retMembers,
                              std::vector<double> &retScores);

    Error ZREVRANGEBYSCORE(
        const std::string &key,
        const double max,
        const double min,
        std::vector<std::string> &retMembers);
    Error ZREVRANGEBYSCORE(
        const std::string &key,
        const double max,
        const double min,
        const long offset,
        const long count,
        std::vector<std::string> &retMembers);

    // count 参考值为10 ~ 1000
    Error ZSCAN(const std::string &key, const long cursor, const std::string &pattern, const long count,
                long &retCursor, std::vector<std::string> &retMembers, std::vector<double> &retScores);

public:
    // Script 脚本, 命令参考: https://www.runoob.com/redis/redis-scripting.html

    Error EVAL(const std::string &script,
               const std::vector<std::string> &keys,
               const std::vector<std::string> &args,
               std::vector<std::string> &retVals);

    Error EVALSHA(const std::string &sha1,
                  const std::vector<std::string> &keys,
                  const std::vector<std::string> &args,
                  std::vector<std::string> &retVals);

    Error SCRIPT_EXISTS(const std::vector<std::string> &sha1s, std::vector<long> &retVals);
    Error SCRIPT_FLUSH();
    Error SCRIPT_KILL();
    Error SCRIPT_LOAD(const std::string &script, std::string &retVal);

private:
    // 传递进来的参数是数组, 命令以string作为分割, 需要format之后再执行调用.
    void *Command(const std::string &fmtcmd);
    void *CommandExec(const std::string &fmtcmd);

    // pipe接口
    int pipeCommand(const std::string &fmtcmd);
    void *getPipeReply();

    // command 通过这四个函数实现命令统一操作
    inline void cleanCommand() noexcept
    {
        // 需要调用两个函数完成清理
        m_ssCMD.str("");
        m_ssCMD.clear();
    }
    inline void setCommandNum(const int cmdNum)
    {
        m_ssCMD << "*" << cmdNum << "\r\n";
    }
    inline void addCommandParam(const std::string &cmdItem)
    {
        m_ssCMD << "$" << cmdItem.length() << "\r\n";
        m_ssCMD << cmdItem << "\r\n";
    }
    inline std::string getCommand()
    {
        return m_ssCMD.str();
    }

private:
    TDRedis()
    {
        m_pRedisContext = nullptr;
        m_nPort = 0;
        m_nIndex = 0;
        m_nTimeout = 0;
        m_nRequestCount = 0;
        m_vecErrorInfo.reserve(2);
        m_errCode = Error::OK;
        cleanCommand();
    }
    ~TDRedis() { DisconnectServer(); }
    TDRedis(const TDRedis &) = delete;
    TDRedis &operator=(const TDRedis &) = delete;

    // 超时时间timeout建议100ms
    Error ConnectServer(
        const std::string &name,
        const std::string &ip,
        const int port,
        const std::string &password,
        const int index,
        const int timeout);
    Error DisconnectServer();
    Error ReconnectServer();

    // 账号密码认证, 仅在连接时使用.
    Error AUTH(const std::string &password);

    // 华为云Redis集群不支持SELECT命令, 分库需求建议拆分使用多个Redis服务.
    // 支持指定数据库的情况下, 需要考虑连接池拿出连接执行SELECT导致的库切换问题, 故禁止外部调用
    // 解决方案: 连接时指定数据库. 多库采用多连接队列.
    Error SELECT(int index);

private:
    redisContext *m_pRedisContext;
    std::string m_strName;
    std::string m_strIP;
    int m_nPort;
    std::string m_strPassword;
    int m_nIndex;
    int m_nTimeout;

    int m_nRequestCount;
    std::vector<std::string> m_vecErrorInfo; // push_back, back, pop_back当栈使用
    Error m_errCode;                         // 错误码, 仅记录上一次请求结果
    std::stringstream m_ssCMD;               // 拼接命令字符串使用
};
