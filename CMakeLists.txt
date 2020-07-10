cmake_minimum_required(VERSION 3.16)

project(100GB LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)

# set(CMAKE_BUILD_TYPE Debug)

find_package(Threads REQUIRED)

add_executable(main src/main.cc)
target_include_directories(main PRIVATE src/include)
target_link_libraries(main PRIVATE Threads::Threads)

add_executable(naive src/naive.cc)
target_include_directories(naive PRIVATE src/include)

macro(add_unittest test)
  add_executable(${test} test/test_${test}.cc)
  target_include_directories(${test} PRIVATE src/include)
  target_link_libraries(${test} PRIVATE Threads::Threads)
  add_test(${test} ${test})
endmacro()

enable_testing()
add_unittest(words_reader)
add_unittest(words_writer)
add_unittest(partition)
