aux_source_directory(. LIB_SRCS)
aux_source_directory(./tensorflow_serving/apis LIB_SRCS)
aux_source_directory(./tensorflow/core/framework LIB_SRCS)
add_library(TFServingProtos STATIC ${LIB_SRCS})

# include directories
INCLUDE_DIRECTORIES(
  ./
)