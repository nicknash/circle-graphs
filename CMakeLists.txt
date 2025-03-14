cmake_minimum_required(VERSION 3.10.0)
project(circle-graphs VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
add_executable(circle-graphs main.cpp)

target_include_directories(circle-graphs
    PRIVATE src              
    PUBLIC include           
)

target_compile_features(circle-graphs PRIVATE cxx_std_20)

