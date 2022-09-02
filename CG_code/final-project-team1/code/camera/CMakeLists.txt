aux_source_directory(. DIR_LIB_SRCS)

add_library(camera ${DIR_LIB_SRCS})

target_link_libraries(camera geometry)