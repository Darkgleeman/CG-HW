
aux_source_directory(. DIR_LIB_SRCS)

add_library(aggregate ${DIR_LIB_SRCS})

if(INTEL)
    target_link_libraries(aggregate embree)
endif()