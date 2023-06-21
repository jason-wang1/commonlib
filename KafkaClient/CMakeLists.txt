aux_source_directory(. LIB_SRCS)
aux_source_directory(crypto LIB_CRYPTO)
add_library(KafkaClient STATIC ${LIB_SRCS} ${LIB_CRYPTO})

TARGET_LINK_LIBRARIES(
    KafkaClient
    rdkafka
    rdkafka++
    rdkafka-static
    glog
    pthread
    dl
    ssl
    crypto
    sasl2
    z
    zstd
    gssapi_krb5
)
