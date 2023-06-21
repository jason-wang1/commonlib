aux_source_directory(. LIB_SRCS)
add_library(LarkMessage STATIC ${LIB_SRCS})

TARGET_LINK_LIBRARIES(
    LarkMessage
    curl
    ssl
    crypto
    nghttp2
)
