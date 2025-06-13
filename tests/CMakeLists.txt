file(GLOB TEST_SOURCES "*.cpp")

add_executable(circle-graphs-tests ${TEST_SOURCES})

target_include_directories(circle-graphs-tests
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(circle-graphs-tests PRIVATE circle-graphs-lib)

target_compile_features(circle-graphs-tests PRIVATE cxx_std_23)

add_test(NAME circle-graphs-tests COMMAND circle-graphs-tests)
