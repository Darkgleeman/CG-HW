aux_source_directory(. DIR_LIB_SRCS)

add_library(geometry ${DIR_LIB_SRCS})

target_link_libraries(geometry medium)
if(INTEL)
    target_link_libraries(geometry embree)
endif()