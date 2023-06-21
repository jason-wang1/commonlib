aux_source_directory(. LIB_SRCS)
add_library(RegisterCenter STATIC ${LIB_SRCS})

TARGET_LINK_LIBRARIES(
    RegisterCenter
    Protobuf
    GrpcDispatcher
    MurmurHash3
    glog
)
