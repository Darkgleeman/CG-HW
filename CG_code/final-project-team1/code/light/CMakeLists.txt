aux_source_directory(. DIR_LIB_SRCS)

add_library(light ${DIR_LIB_SRCS})

if(INTEL)
    target_link_libraries(light embree)
endif()