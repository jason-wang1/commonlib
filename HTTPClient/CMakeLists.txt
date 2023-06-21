aux_source_directory(. LIB_SRCS)
add_library(HTTPClient STATIC ${LIB_SRCS})

TARGET_LINK_LIBRARIES(
    HTTPClient
    curl
    ssl
    crypto
    nghttp2
    pthread
)
