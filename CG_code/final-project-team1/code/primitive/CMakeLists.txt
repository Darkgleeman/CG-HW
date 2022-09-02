aux_source_directory(. DIR_LIB_SRCS)

add_library(primitive ${DIR_LIB_SRCS})

if(INTEL)
    target_link_libraries(primitive embree)
endif()