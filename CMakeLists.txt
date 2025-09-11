cmake_minimum_required(VERSION 3.14)
project(circle-graphs VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put all executables in the source dir (your current choice)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# ---- library & app ----------------------------------------------------------
file(GLOB_RECURSE LIB_SRC_FILES "src/*.cpp")
list(REMOVE_ITEM LIB_SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_library(circle-graphs-lib ${LIB_SRC_FILES})

add_executable(circle-graphs src/main.cpp)
target_link_libraries(circle-graphs PRIVATE circle-graphs-lib)

target_include_directories(circle-graphs-lib PRIVATE src PUBLIC include)
target_compile_features(circle-graphs-lib PRIVATE cxx_std_23)
target_compile_features(circle-graphs      PRIVATE cxx_std_23)

# Default to Debug for nicer stepping in tests
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---- testing / doctest ------------------------------------------------------
include(CTest)  # adds BUILD_TESTING option

include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

# Make doctest.cmake visible to subdirs (optional; we also include by path there)
list(APPEND CMAKE_MODULE_PATH "${doctest_SOURCE_DIR}/scripts/cmake")

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
