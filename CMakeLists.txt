cmake_minimum_required(VERSION 3.14)
project(g200-pg-decoder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library
set(SOURCES
    src/pg_common.cpp
    src/pg_type0_decoder.cpp
    src/pg_type1_decoder.cpp
    src/pg_decoder.cpp
)

add_library(pg_decoder STATIC ${SOURCES})
target_include_directories(pg_decoder PUBLIC include)

# Tests
enable_testing()
add_subdirectory(tests)
