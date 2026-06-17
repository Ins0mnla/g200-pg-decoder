cmake_minimum_required(VERSION 3.14)

set(TEST_SOURCES
    test_main.cpp
    test_type0.cpp
    test_type1.cpp
)

add_executable(test_pg_decoder ${TEST_SOURCES})
target_link_libraries(test_pg_decoder PRIVATE pg_decoder)
target_include_directories(test_pg_decoder PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME pg_decoder_tests COMMAND test_pg_decoder)
