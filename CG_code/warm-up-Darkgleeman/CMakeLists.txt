cmake_minimum_required(VERSION 3.16)
project(CS171-hw0 DESCRIPTION "warm up homework" LANGUAGES C CXX)

# glfw
option(GLFW_BUILD_TESTS off)
option(GLFW_BUILD_EXAMPLES off)
option(GLFW_BUILD_DOCS off)
add_subdirectory(libs/glfw)
list(APPEND INC_DIRS libs/glfw/include)
list(APPEND LIBRARIES glfw ${glfw_LIBRARIES})

# glew
option(BUILD_UTILS off)
add_subdirectory(libs/glew/build/cmake)
list(APPEND INC_DIRS libs/glew/include)
list(APPEND LIBRARIES glew_s)

list(APPEND INC_DIRS libs/glm)

list(APPEND INC_DIRS include)

# set source files and add executable
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(main ${SRC_FILES})
target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PRIVATE ${INC_DIRS})
# link libraries
target_link_libraries(main PRIVATE ${LIBRARIES})