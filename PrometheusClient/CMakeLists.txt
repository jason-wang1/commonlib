aux_source_directory(. LIB_SRCS)
add_library(PrometheusClient STATIC ${LIB_SRCS})

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/CommonLib/third_party/prometheus-cpp/core/include/
  ${PROJECT_SOURCE_DIR}/CommonLib/third_party/prometheus-cpp/push/include/
)

TARGET_LINK_LIBRARIES(
  PrometheusClient
  prometheus-cpp-push
  prometheus-cpp-core
  glog
)
