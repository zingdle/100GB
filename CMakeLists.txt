cmake_minimum_required(VERSION 3.16)

project(100GB LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)

find_package(Threads REQUIRED)

add_executable(main src/main.cc)
target_include_directories(main PRIVATE src/include)

add_executable(naive src/naive.cc)
target_include_directories(naive PRIVATE src/include)

macro(add_test test)
  add_executable(${test} test/${test}.cc)
  target_include_directories(${test} PRIVATE src/include)
endmacro()

add_test(test_words_reader)
add_test(test_words_writer)
add_test(test_partition)
