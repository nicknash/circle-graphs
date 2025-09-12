# tests_main.cpp should live at tests/tests_main.cpp
#   with:
#     #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#     #include <doctest/doctest.h>

# Recursively collect test sources in subdirectories too
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*.cc"
  "${CMAKE_CURRENT_LIST_DIR}/*.cxx"
)

# Make sure tests_main is compiled exactly once
set(TEST_MAIN "${CMAKE_CURRENT_LIST_DIR}/test_main.cpp")
list(REMOVE_ITEM TEST_SOURCES "${TEST_MAIN}")

# Helpful log while you’re wiring things up
message(STATUS "unit_tests sources:")
foreach(s IN LISTS TEST_SOURCES)
  message(STATUS "  ${s}")
endforeach()

add_executable(unit_tests ${TEST_MAIN} ${TEST_SOURCES})

target_link_libraries(unit_tests
  PRIVATE
    circle-graphs-lib
    doctest::doctest
)

target_include_directories(unit_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(unit_tests PRIVATE cxx_std_23)

# Include doctest's CMake helpers explicitly (robust, no PATH surprises)
include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")

# Register EACH TEST_CASE so VS Code can run/debug them individually
doctest_discover_tests(unit_tests
  TEST_PREFIX ""
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DISCOVERY_MODE POST_BUILD   # discover immediately after the binary is built
)
