
#include "gtest/gtest.h"
#include "linedet.hpp"
#include "arrays.hpp"

TEST (ConvolveAverageTests, ArbitraryNumbers) {
    auto array = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    fill2DArray<float>(array, NUM_ROTATIONS, ACC_SIZE, 0);

    auto result = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    fill2DArray<float>(result, NUM_ROTATIONS, ACC_SIZE, 0);

    int filterSize = 3;
    // Set the array to: [[0,0,3,3,6,0,0,...0], [0...0], ...]
    // Which should result in: [[0,1,2,4,3,2,0,...0], [0...0], ...]
    array[0][2] = 3;
    array[0][3] = 3;
    array[0][4] = 6;

    convolveAverage(array, filterSize, result);

    unsigned expected_results[][2] = {{0, 0},
                                      {1, 1},
                                      {2, 2},
                                      {3, 4},
                                      {4, 3},
                                      {5, 2},
                                      {6, 0}};

    for (auto &expected_result : expected_results) {
        unsigned index = expected_result[0];
        unsigned value = expected_result[1];
        ASSERT_EQ(result[0][index], value);
    }

    delete2DArray(array, NUM_ROTATIONS);
    delete2DArray(result, NUM_ROTATIONS);
}

TEST(ExtractingPeaksTests, PositiveAndNegativeDiffs) {

    auto acc = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    auto average = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    auto peaks = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    fill2DArray<float>(acc, NUM_ROTATIONS, ACC_SIZE, 0);
    fill2DArray<float>(average, NUM_ROTATIONS, ACC_SIZE, 0);

    acc[0][0] = 10;
    acc[0][1] = 15;
    acc[0][2] = 15;

    average[0][0] = 15;
    average[0][1] = 10;
    average[0][2] = 15;

    extractPeaks(acc, average, peaks);

    EXPECT_EQ(peaks[0][0], 0); // Neg - Pos => 0
    EXPECT_EQ(peaks[0][1], 5); // Pos - Neg => Pos
    EXPECT_EQ(peaks[0][2], 0); // Val - Val => 0
    EXPECT_EQ(peaks[0][3], 0); // For zeros

    delete2DArray(acc, NUM_ROTATIONS);
    delete2DArray(average, NUM_ROTATIONS);
    delete2DArray(peaks, NUM_ROTATIONS);
}

TEST(ExtractingSlopes, SmallWindowOnTheLeftSideOfArray) {
    auto array = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    auto slopes = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    fill2DArray<float>(array, NUM_ROTATIONS, ACC_SIZE, 0);
    int sideDistance = 2;

    array[0][1] = 1;
    array[0][2] = 10;
    array[0][4] = 4;

    extractSlopes(array, sideDistance, slopes);

    // Values at the beginning and the end should
    // be set to a safe value
    float maxValue = std::numeric_limits<float>::max();
    unsigned expected_results[][2] = {{0, maxValue},
                                      {1, maxValue},
                                      {ACC_SIZE - 1, maxValue},
                                      {ACC_SIZE - 2, maxValue},
                                      {2, 4},
                                      {3, 1},
                                      {4, 10},
                                      {5, 0},
                                      {6, 4},
                                      {7, 0}};

    for (auto &expected_result : expected_results) {
        unsigned index = expected_result[0];
        unsigned value = expected_result[1];
        EXPECT_EQ(slopes[0][index], value);
    }
    delete2DArray(array, NUM_ROTATIONS);
    delete2DArray(slopes, NUM_ROTATIONS);
}

TEST(SelectingPeakTests, ExpectedBehaviour) {
    auto peaks = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    auto slopes = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    auto selectedPeaks = allocate2DArray<float>(NUM_ROTATIONS, 2);
    fill2DArray<float>(peaks, NUM_ROTATIONS, ACC_SIZE, 0);
    fill2DArray<float>(slopes, NUM_ROTATIONS, ACC_SIZE, 0);
    fill2DArray<float>(selectedPeaks, NUM_ROTATIONS, 2, 0);
    unsigned slopeThreshold = 10;

    peaks[0][0] = 1;
    peaks[0][1] = 80;
    peaks[0][2] = 5;
    peaks[0][3] = 9;
    peaks[0][4] = 7;

    slopes[0][0] = 10;
    slopes[0][1] = 15;
    slopes[0][2] = 5;
    slopes[0][3] = 8;
    slopes[0][4] = 14;
    slopes[0][5] = 0;

    selectPeaksUsingSlopes(peaks, slopes, slopeThreshold, selectedPeaks);

    EXPECT_EQ(selectedPeaks[0][0], 3); // column
    EXPECT_EQ(selectedPeaks[0][1], 9); // value

    for (int rot = 1; rot < NUM_ROTATIONS; rot++) {
        EXPECT_EQ(selectedPeaks[rot][1], 0); // value
    }
    delete2DArray(peaks, NUM_ROTATIONS);
    delete2DArray(slopes, NUM_ROTATIONS);
    delete2DArray(selectedPeaks, NUM_ROTATIONS);
}