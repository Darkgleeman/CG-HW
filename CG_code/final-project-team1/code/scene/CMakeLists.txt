aux_source_directory(. DIR_LIB_SRCS)

add_library(scene ${DIR_LIB_SRCS})

target_link_libraries(scene spectrum)
if(INTEL)
    target_link_libraries(scene embree)
endif()