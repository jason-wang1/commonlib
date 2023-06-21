#pragma once
#include <string>
#include <vector>
#include <future>
#include <sys/time.h>
#include <unordered_set>
#include "librdkafka/rdkafkacpp.h"

// Kafka 错误码
enum Error
{
    OK = 0,
    NormalError = 1000,   // Kafka常规错误
    ConfCreateFailed,     // 配置创建失败
    GlobalConfError,      // 全局配置错误
    TopicConfError,       // Topic配置错误
    ProducerCreateFailed, // 生产者创建失败
    TopicCreateFailed,    // Topic创建失败
    PushDataFailed,       // 推送数据失败
};

// Kafka 数据
struct TDKafkaData
{
    std::string topic_id = "";
    int partition = RdKafka::Topic::PARTITION_UA;

    std::string msg = "";
};

// Kafka 配置
struct TDKafkaConfig
{
    std::vector<std::string> m_VecBrokers; // BroKers
    bool m_IsUseSasl = false;              // 是否使用Sasl
    std::string m_SecurityProtocol = "";   // 安全协议
    std::string m_SaslMechanism = "";      // Sasl机制
    std::string m_SaslUserName = "";       // Sasl用户名
    std::string m_SaslPassword = "";       // Sasl密码
};

class TDKafkaDeliveryReportCb : public RdKafka::DeliveryReportCb
{
public:
    void dr_cb(RdKafka::Message &message);
};

class TDKafkaProducer
{
public:
    TDKafkaProducer() {}
    virtual ~TDKafkaProducer() {}

public:
    int Init(
        const TDKafkaConfig &kafka_conf);

    void ShutDown();

    int PushData(const TDKafkaData &data);

private:
    TDKafkaConfig m_KafkaConf;

    std::shared_ptr<RdKafka::Conf> m_spConf;
    std::shared_ptr<RdKafka::Producer> m_spProducer;
    std::shared_ptr<TDKafkaDeliveryReportCb> m_spDeliveryReportCb;
};
