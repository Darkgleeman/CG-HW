aux_source_directory(. DIR_LIB_SRCS)

add_library(distribution ${DIR_LIB_SRCS})

if(INTEL)
    target_link_libraries(distribution embree)
endif()