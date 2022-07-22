# 'Google_test' is the subproject name
project(google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(Google_Tests_run linedet_tests.cpp coordinates_tests.cpp)
target_link_libraries(Google_Tests_run gtest gtest_main ${OpenCV_LIBS} rotlinedet_lib)
