aux_source_directory(. DIR_LIB_SRCS)

add_library(sampler ${DIR_LIB_SRCS})

target_link_libraries(sampler geometry)