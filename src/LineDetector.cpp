//
// Created by lada on 7/25/22.
//

#include "LineDetector.hpp"
#include <chrono>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std::chrono;

LineDetector::LineDetector(std::string &pixelCountFilePath, int averagingFilterSize,
                           int minPixelsThreshold, float slopeThreshold, int candidates,
                           bool verbosely) :
        averagingFilterSize(averagingFilterSize), minPixelsThreshold(minPixelsThreshold),
        slopeThreshold(slopeThreshold), candidates(candidates), verbosely(verbosely) {

    sideDistance = averagingFilterSize / 2;
    columnPixelCounts = loadColumnPixelCountsFromFile(pixelCountFilePath);

    acc = allocate2DArray<unsigned>(NUM_ROTATIONS, ACC_SIZE);
    normalizedAcc = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    average = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    peaks = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    slopes = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);

}

LineDetector::~LineDetector() {
    delete2DArray(columnPixelCounts, NUM_ROTATIONS);
    delete2DArray(acc, NUM_ROTATIONS);
    delete2DArray(normalizedAcc, NUM_ROTATIONS);
    delete2DArray(average, NUM_ROTATIONS);
    delete2DArray(peaks, NUM_ROTATIONS);
    delete2DArray(slopes, NUM_ROTATIONS);
}

std::vector<image_endpoints_t> LineDetector::processImage(std::vector<unsigned char> &imageVectorData) {
//    auto start = high_resolution_clock::now();

    fill2DArray<unsigned>(acc, NUM_ROTATIONS, ACC_SIZE, 0);
    fill2DArray<float>(average, NUM_ROTATIONS, ACC_SIZE, 0);

    setImgData(imageVectorData);

//    auto stop = high_resolution_clock::now();
//    auto duration = duration_cast<milliseconds>(stop - start);
//    std::cout << "Duration: " << duration.count() << " ms" << std::endl;

    sumColumns(imgData, rotations, acc);

    normalizeAccByNumPixels(acc, columnPixelCounts, normalizedAcc);
    convolveAverage(normalizedAcc, averagingFilterSize, average);
    extractPeaks(normalizedAcc, average, peaks);
    extractSlopes(average, sideDistance, slopes);
    ignoreColumnsWithTooFewPixels(slopes, columnPixelCounts, minPixelsThreshold);

    auto selectedPeaks = selectPeaksUsingSlopes(peaks, slopes, slopeThreshold);
    moveBestPeaksToFront(selectedPeaks);
    auto imageEndpoints = determineImageEndpoints(selectedPeaks);

    return imageEndpoints;
}

void LineDetector::moveBestPeaksToFront(std::vector<Peak> &peaks) const {
    std::nth_element(peaks.begin(), peaks.begin() + candidates, peaks.end(),
                     [](const Peak &lhs, const Peak &rhs) {
                         return lhs.value > rhs.value;
                     });
}

std::vector<image_endpoints_t> LineDetector::determineImageEndpoints(
        std::vector<Peak> peaks) const {
    std::vector<image_endpoints_t> endpoints;
    endpoints.reserve(candidates);

    for (int i = 0; i < candidates; i++) {
        auto peak = peaks.at(i);
        image_dimensions_t imageShape = {.width = IMG_WIDTH, .height = IMG_HEIGHT};
        image_dimensions_t paddedImageShape = {.width = ACC_SIZE, .height = ACC_SIZE};
        slope_intercept_line_t line = lineFromAngleAndCol(
                peak.getRotationAngle(), peak.column, imageShape, paddedImageShape);
        image_endpoints_t imageEndpoints = lineEndPointsOnImage(line, imageShape);
        endpoints.push_back(imageEndpoints);
    }
    return endpoints;
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

void LineDetector::setImgData(std::vector<unsigned char> &vector) {
    for (int row = 0; row < IMG_HEIGHT; row++) {
        for (int col = 0; col < IMG_WIDTH; col++) {
            imgData[row][col] = vector.at(row * IMG_WIDTH + col);
        }
    }
}

