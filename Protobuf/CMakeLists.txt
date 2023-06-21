aux_source_directory(. LIB_SRCS)
aux_source_directory(./redis LIB_SRCS)
aux_source_directory(./other LIB_SRCS)
aux_source_directory(./proxy LIB_SRCS)
add_library(Protobuf STATIC ${LIB_SRCS})

# include directories
INCLUDE_DIRECTORIES(
  ./
)
