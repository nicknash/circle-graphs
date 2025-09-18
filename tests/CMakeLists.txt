# test_main.cpp should live at tests/test_main.cpp
#   with:
#     #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#     #include <doctest/doctest.h>

# Recursively collect test sources in subdirectories too
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*.cc"
  "${CMAKE_CURRENT_LIST_DIR}/*.cxx"
)

# Make sure test_main is compiled exactly once
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

include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")

doctest_discover_tests(unit_tests
  TEST_PREFIX "ut."
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DISCOVERY_MODE POST_BUILD
  DISCOVERY_TIMEOUT 60
  EXTRA_ARGS --order-by=file --no-intro
)
