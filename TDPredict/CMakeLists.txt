aux_source_directory(. LIB_SRCS)
aux_source_directory(./Config LIB_SRCS)
aux_source_directory(./Feature LIB_SRCS)
aux_source_directory(./FMModel LIB_SRCS)
aux_source_directory(./Interface LIB_SRCS)
aux_source_directory(./RankCalc LIB_SRCS)
aux_source_directory(./Strategy LIB_SRCS)
aux_source_directory(./TFModel LIB_SRCS)
add_library(TDPredict STATIC ${LIB_SRCS})

# include directories
INCLUDE_DIRECTORIES(
  ./
  ${PROJECT_SOURCE_DIR}/CommonLib/TDPredict
  ${PROJECT_SOURCE_DIR}/CommonLib/TFServingProtos
  ${PROJECT_SOURCE_DIR}/CommonLib/third_party/eigen3/include
)

TARGET_LINK_LIBRARIES(
  TDPredict
  HTTPClient
  TFServingProtos
  glog
  PrometheusClient
)
