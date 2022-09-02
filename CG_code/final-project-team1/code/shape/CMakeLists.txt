aux_source_directory(. DIR_LIB_SRCS)

add_library(shape ${DIR_LIB_SRCS})

if(INTEL)
    target_link_libraries(shape embree)
endif()