cmake_minimum_required(VERSION 3.16)
project(rotlinedet)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(include)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(google_tests)