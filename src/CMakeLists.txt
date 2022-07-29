cmake_minimum_required(VERSION 3.16)
project(rotlinedet)

set(CMAKE_CXX_STANDARD 17)
set(BINARY ${CMAKE_PROJECT_NAME})


file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
list(REMOVE_ITEM SOURCES "test_columnsum.cpp")
set(SOURCES ${SOURCES})
add_executable(${BINARY}_run ${SOURCES})
add_library(${BINARY}_lib SHARED ${SOURCES})

target_link_libraries(${BINARY}_run)
