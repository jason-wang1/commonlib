aux_source_directory(. LIB_SRCS)
add_library(FMModel STATIC ${LIB_SRCS})

TARGET_LINK_LIBRARIES(
    FMModel
    glog
)
