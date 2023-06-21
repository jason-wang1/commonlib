aux_source_directory(. LIB_SRCS)
add_library(ProtoFactory STATIC ${LIB_SRCS})

TARGET_LINK_LIBRARIES(
    ProtoFactory
    ${_PROTOBUF_LIBPROTOBUF}
)
