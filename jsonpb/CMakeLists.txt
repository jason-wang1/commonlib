aux_source_directory(. LIB_SRCS)
add_library(jsonpb STATIC ${LIB_SRCS})

TARGET_LINK_LIBRARIES(
    jsonpb
    ${_PROTOBUF_LIBPROTOBUF}
)
