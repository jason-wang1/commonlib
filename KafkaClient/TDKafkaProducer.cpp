#include "TDKafkaProducer.h"
#include "glog/logging.h"
#include "Common/Function.h"
#include "crypto/crypto.h"

void TDKafkaDeliveryReportCb::dr_cb(RdKafka::Message &message)
{
    RdKafka::ErrorCode error_code = message.err();
    if (error_code == RdKafka::ErrorCode::ERR_NO_ERROR)
    {
        // LOG(INFO) << "TDKafkaDeliveryReportCb::dr_cb(): Producer send success, delivery " << message.len()
        //           << " bytes, [" << message.partition()
        //           << "] at offset " << message.offset();
    }
    else
    {
        LOG(WARNING) << "TDKafkaDeliveryReportCb::dr_cb(): Message Error"
                     << ", error_code = " << error_code
                     << ", error_str = " << message.errstr()
                     << ", partition = " << message.partition();
    }
}

int TDKafkaProducer::Init(
    const TDKafkaConfig &kafka_conf)
{
    // 是否传入 Brokers
    if (kafka_conf.m_VecBrokers.size() == 0)
    {
        return Error::NormalError;
    }

    m_KafkaConf = kafka_conf;

    // 错误信息
    std::string error_info = "";

    // 创建配置
    m_spConf = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    if (m_spConf == nullptr || m_spConf.get() == nullptr)
    {
        LOG(ERROR) << "TDKafkaProducer::Init(): Global config create failed.";
        return Error::ConfCreateFailed;
    }

    std::string str_servers = m_KafkaConf.m_VecBrokers[0];
    for (size_t idx = 1, count = m_KafkaConf.m_VecBrokers.size(); idx < count; idx++)
    {
        str_servers += ", " + m_KafkaConf.m_VecBrokers[idx];
    }

    // 配置 - bootstrap.servers
    RdKafka::Conf::ConfResult conf_result = RdKafka::Conf::CONF_UNKNOWN;
    conf_result = m_spConf->set("bootstrap.servers", str_servers, error_info);
    if (RdKafka::Conf::CONF_OK != conf_result)
    {
        LOG(ERROR) << "TDKafkaProducer::Init(): Config bootstrap.servers failed"
                   << ", str_servers = " << str_servers
                   << ", conf_result = " << conf_result
                   << ", error_info = " << error_info;
        return Error::GlobalConfError;
    }

    m_spDeliveryReportCb = std::make_shared<TDKafkaDeliveryReportCb>();

    conf_result = RdKafka::Conf::CONF_UNKNOWN;
    conf_result = m_spConf->set("dr_cb", m_spDeliveryReportCb.get(), error_info);
    if (RdKafka::Conf::CONF_OK != conf_result)
    {
        LOG(ERROR) << "TDKafkaProducer::Init(): Config dr_cb failed"
                   << ", conf_result = " << conf_result
                   << ", error_info = " << error_info;
        return Error::GlobalConfError;
    }

    conf_result = RdKafka::Conf::CONF_UNKNOWN;
    conf_result = m_spConf->set("compression.codec", "gzip", error_info);
    if (RdKafka::Conf::CONF_OK != conf_result)
    {
        LOG(ERROR) << "TDKafkaProducer::Init(): Config compression.codec failed"
                   << ", conf_result = " << conf_result
                   << ", error_info = " << error_info;
        return Error::GlobalConfError;
    }

    // 支持Sasl配置
    if (m_KafkaConf.m_IsUseSasl)
    {
        // 配置 - security.protocol
        conf_result = m_spConf->set("security.protocol", m_KafkaConf.m_SecurityProtocol, error_info);
        if (RdKafka::Conf::CONF_OK != conf_result)
        {
            LOG(ERROR) << "TDKafkaProducer::Init(): Config security.protocol failed"
                       << ", security_protocol = " << m_KafkaConf.m_SecurityProtocol
                       << ", conf_result = " << conf_result
                       << ", error_info = " << error_info;
            return Error::GlobalConfError;
        }

        // 配置 - sasl.mechanism
        conf_result = m_spConf->set("sasl.mechanism", m_KafkaConf.m_SaslMechanism, error_info);
        if (RdKafka::Conf::CONF_OK != conf_result)
        {
            LOG(ERROR) << "TDKafkaProducer::Init(): Config sasl.mechanism failed"
                       << ", sasl_mechanism = " << m_KafkaConf.m_SaslMechanism
                       << ", conf_result = " << conf_result
                       << ", error_info = " << error_info;
            return Error::GlobalConfError;
        }

        // 配置 - sasl.username
        conf_result = m_spConf->set("sasl.username", m_KafkaConf.m_SaslUserName, error_info);
        if (RdKafka::Conf::CONF_OK != conf_result)
        {
            LOG(ERROR) << "TDKafkaProducer::Init(): Config sasl.username failed"
                       << ", sasl_username = " << m_KafkaConf.m_SaslUserName
                       << ", conf_result = " << conf_result
                       << ", error_info = " << error_info;
            return Error::GlobalConfError;
        }

        // 配置 - sasl.password
        std::string saslPassword = Crypto::aes_decrypt(m_KafkaConf.m_SaslPassword);
        conf_result = m_spConf->set("sasl.password", saslPassword, error_info);
        if (RdKafka::Conf::CONF_OK != conf_result)
        {
            LOG(ERROR) << "TDKafkaProducer::Init(): Config sasl.password failed"
                       << ", sasl_password = " << m_KafkaConf.m_SaslPassword
                       << ", conf_result = " << conf_result
                       << ", error_info = " << error_info;
            return Error::GlobalConfError;
        }
    }

    // 生产者
    m_spProducer = std::shared_ptr<RdKafka::Producer>(RdKafka::Producer::create(m_spConf.get(), error_info));
    if (m_spProducer == nullptr || m_spProducer.get() == nullptr)
    {
        LOG(ERROR) << "TDKafkaProducer::Init(): Producer create failed"
                   << ", error_info = " << error_info;
        return Error::ProducerCreateFailed;
    }

    return Error::OK;
}

void TDKafkaProducer::ShutDown()
{
    m_spProducer->flush(1000);
    RdKafka::wait_destroyed(1000);
}

int TDKafkaProducer::PushData(const TDKafkaData &data)
{
    // 推送数据
    long now_time = Common::get_ms_timestamp();
    RdKafka::ErrorCode error_code = RdKafka::ErrorCode::ERR_UNKNOWN;
    error_code = m_spProducer->produce(
        data.topic_id, data.partition, RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(data.msg.c_str()),
        data.msg.size(), nullptr, 0, now_time, nullptr);
    if (error_code != RdKafka::ErrorCode::ERR_NO_ERROR)
    {
        LOG(WARNING) << "TDKafkaProducer::PushData(): Producer failed"
                     << ", topic_id = " << data.topic_id
                     << ", msg = " << data.msg
                     << ", partition = " << data.partition
                     << ", error_code = " << error_code;
    }

    m_spProducer->poll(0);
    return Error::OK;
}
