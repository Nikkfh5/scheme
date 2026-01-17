cmake_minimum_required(VERSION 3.16)
project(scheme LANGUAGES CXX)

option(SCHEME_BUILD_CLI "Build CLI executable" OFF)
option(SCHEME_BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(scheme_lib
    parser.cpp
    scheme.cpp
    tokenizer.cpp
)

target_include_directories(scheme_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(scheme_lib PRIVATE /W4)
else()
    target_compile_options(scheme_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (SCHEME_BUILD_CLI)
    add_executable(scheme_cli tools/scheme_cli.cpp)
    target_link_libraries(scheme_cli PRIVATE scheme_lib)
endif()

if (SCHEME_BUILD_TESTS)
    enable_testing()
    add_executable(scheme_tests tests/scheme_tests.cpp)
    target_link_libraries(scheme_tests PRIVATE scheme_lib)
    add_test(NAME scheme_tests COMMAND scheme_tests)
endif()
