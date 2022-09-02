aux_source_directory(. DIR_LIB_SRCS)

add_library(medium ${DIR_LIB_SRCS})

target_link_libraries(medium OpenVDB::openvdb)
if(INTEL)
    target_link_libraries(medium embree)
endif()