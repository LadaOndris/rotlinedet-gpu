//
// Created by lada on 7/25/22.
//

#include "LineDetector.hpp"
#include <chrono>
#include <cmath>
#include <vector>

using namespace std::chrono;

LineDetector::LineDetector(std::string &pixelCountFilePath, int averagingFilterSize,
                           int minPixelsThreshold, float slopeThreshold, bool verbosely) :
        averagingFilterSize(averagingFilterSize), minPixelsThreshold(minPixelsThreshold),
        slopeThreshold(slopeThreshold), verbosely(verbosely) {

    sideDistance = averagingFilterSize / 2;
    columnPixelCounts = loadColumnPixelCountsFromFile(pixelCountFilePath);

    acc = allocate2DArray<unsigned>(NUM_ROTATIONS, ACC_SIZE);
    normalizedAcc = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    average = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    peaks = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    slopes = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    selectedPeaks = allocate2DArray<float>(NUM_ROTATIONS, 2);

}

LineDetector::~LineDetector() {
    delete2DArray(columnPixelCounts, NUM_ROTATIONS);
    delete2DArray(acc, NUM_ROTATIONS);
    delete2DArray(normalizedAcc, NUM_ROTATIONS);
    delete2DArray(average, NUM_ROTATIONS);
    delete2DArray(peaks, NUM_ROTATIONS);
    delete2DArray(slopes, NUM_ROTATIONS);
    delete2DArray(selectedPeaks, NUM_ROTATIONS);
}

image_endpoints_t LineDetector::processImage(std::vector<unsigned char> &imageVectorData) {
//    auto start = high_resolution_clock::now();

    fill2DArray<unsigned>(acc, NUM_ROTATIONS, ACC_SIZE, 0);
    fill2DArray<float>(average, NUM_ROTATIONS, ACC_SIZE, 0);

    setImgData(imageVectorData);

//    auto stop = high_resolution_clock::now();
//    auto duration = duration_cast<milliseconds>(stop - start);
//    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
//
//    start = high_resolution_clock::now();

    sumColumns(imgData, rotations, acc);
//
//    stop = high_resolution_clock::now();
//    duration = duration_cast<milliseconds>(stop - start);
//    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
//

//    start = high_resolution_clock::now();
    normalizeAccByNumPixels(acc, columnPixelCounts, normalizedAcc);
    convolveAverage(normalizedAcc, averagingFilterSize, average);
    extractPeaks(normalizedAcc, average, peaks);
    extractSlopes(average, sideDistance, slopes);
    ignoreColumnsWithTooFewPixels(slopes, columnPixelCounts, minPixelsThreshold);
    selectPeaksUsingSlopes(peaks, slopes, slopeThreshold, selectedPeaks);

//    stop = high_resolution_clock::now();
//    duration = duration_cast<milliseconds>(stop - start);
//    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

//    start = high_resolution_clock::now();

    float rotation, column;
    getMaxPeak(rotation, column);

    image_dimensions_t imageShape = {.width = IMG_WIDTH, .height = IMG_HEIGHT};
    image_dimensions_t paddedImageShape = {.width = ACC_SIZE, .height = ACC_SIZE};
    slope_intercept_line_t line = lineFromAngleAndCol(rotation, column, imageShape, paddedImageShape);
    image_endpoints_t imageEndpoints = lineEndPointsOnImage(line, imageShape);

//    stop = high_resolution_clock::now();
//    duration = duration_cast<milliseconds>(stop - start);
//    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    return imageEndpoints;
}

float **LineDetector::loadColumnPixelCountsFromFile(const std::string &filePath) {
    u_int16_t num_rotations, acc_width;
    std::ifstream fin(filePath, std::ios::binary);
    fin.read(reinterpret_cast<char *>(&num_rotations), sizeof(u_int16_t));
    fin.read(reinterpret_cast<char *>(&acc_width), sizeof(u_int16_t));

    float **values = allocate2DArray<float>(num_rotations, acc_width);
    // The values are saved as 16-bit values, which is enough
    // to serialize values that are up to a few thousand at the most.
    u_int16_t value;

    for (int i = 0; i < num_rotations; i++) {
        for (int j = 0; j < acc_width; j++) {
            fin.read(reinterpret_cast<char *>(&value), sizeof(u_int16_t));
            values[i][j] = static_cast<float>(value);
        }
    }
    return values;
}

void LineDetector::getMaxPeak(float &rotation, float &column) {
    float *bestPeak = selectedPeaks[0];
    for (int i = 0; i < NUM_ROTATIONS; i++) {
        double angleRads = atan(rotations[i][1] / rotations[i][0]);
        double angleDegs = angleRads / M_PI * 180;

        if (selectedPeaks[i][1] > bestPeak[1]) {
            bestPeak = selectedPeaks[i];
            rotation = angleRads;
        }
        if (verbosely) {
            std::cout << i << " (" << angleDegs << "): " << selectedPeaks[i][1] << " at " <<
                      selectedPeaks[i][0] << std::endl;
        }
    }
    if (verbosely) {
        std::cout << "Col: " << bestPeak[0] << ", metric: " << bestPeak[1] << ", rotation: " << rotation / M_PI * 180
                  << "°"
                  << std::endl;
    }
    column = bestPeak[0];
}

void LineDetector::setImgData(std::vector<unsigned char> &vector) {
    for (int row = 0; row < IMG_HEIGHT; row++) {
        for (int col = 0; col < IMG_WIDTH; col++) {
            imgData[row][col] = vector.at(row * IMG_WIDTH + col);
        }
    }
}

