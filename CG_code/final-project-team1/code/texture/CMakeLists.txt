aux_source_directory(. DIR_LIB_SRCS)

add_library(texture ${DIR_LIB_SRCS})

if(INTEL)
    target_link_libraries(texture embree)
endif()