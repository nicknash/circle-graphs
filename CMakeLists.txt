cmake_minimum_required(VERSION 3.10.0)
project(circle-graphs VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})


file(GLOB_RECURSE LIB_SRC_FILES "src/*.cpp")
list(REMOVE_ITEM LIB_SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_library(circle-graphs-lib ${LIB_SRC_FILES})

add_executable(circle-graphs src/main.cpp)
target_link_libraries(circle-graphs PRIVATE circle-graphs-lib)

set(CMAKE_BUILD_TYPE Debug)

target_include_directories(circle-graphs-lib
    PRIVATE src
    PUBLIC include
)
target_compile_features(circle-graphs-lib PRIVATE cxx_std_23)

target_compile_features(circle-graphs PRIVATE cxx_std_23)

enable_testing()
add_subdirectory(tests)

