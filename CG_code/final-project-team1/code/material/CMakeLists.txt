aux_source_directory(. DIR_LIB_SRCS)

add_library(material ${DIR_LIB_SRCS})

target_link_libraries(material utils)
target_link_libraries(material scattering)
target_link_libraries(material texture)