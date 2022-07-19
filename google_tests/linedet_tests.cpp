
#include "gtest/gtest.h"
#include "linedet.hpp"

TEST (ConvolveAverageTests, ArbitraryNumbers) {
    unsigned int array[NUM_ROTATIONS][ACC_SIZE] = {0};
    unsigned int result[NUM_ROTATIONS][ACC_SIZE] = {0};
    int filterSize = 3;
    // Set the array to: [[0,0,3,3,6,0,0,...0], [0...0], ...]
    // Which should result in: [[0,1,2,4,3,2,0,...0], [0...0], ...]
    array[0][2] = 3;
    array[0][3] = 3;
    array[0][4] = 6;

    convolve_average(array, filterSize, result);

    unsigned expected_results[][2] = {{0, 0},
                                          {1, 1},
                                          {2, 2},
                                          {3, 4},
                                          {4, 3},
                                          {5, 2},
                                          {6, 0}};

    for (auto & expected_result : expected_results) {
        unsigned index = expected_result[0];
        unsigned value = expected_result[1];
        ASSERT_EQ(result[0][index], value);
    }

}