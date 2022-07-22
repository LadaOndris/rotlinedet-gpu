#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <omp.h>
#include <openacc.h>
#include "linedet.hpp"
#include <cassert>
#include <iostream>
#include <chrono>
#include "RunParams.hpp"
#include "arrays.hpp"

using namespace std;
using namespace std::chrono;

int parseArgs(int argc, char **argv, RunParams &params) {
    vector<string> args(argv + 1, argv + argc);

    for (std::vector<string>::size_type i = 0; i < args.size(); i += 2) {
        if (i + 1 == args.size()) {
            cout << "Expected parameter value" << endl;
            return 1;
        }
        auto nextArg = args[i + 1].c_str();

        if (args[i] == "--image") {
            params.imagePath = args[i + 1];
        } else if (args[i] == "--filterSize") {
            params.averagingFilterSize = stoi(args[i + 1]);
        } else if (args[i] == "--slopeThreshold") {
            params.slopeThreshold = stof(args[i + 1]);
        } else if (args[i] == "--minPixelsThreshold") {
            params.minPixelsThreshold = stoi(args[i + 1]);
        } else {
            cout << "Unknown option: " << args[i] << endl;
            return 2;
        }
    }
    return 0;
}

void displayImage(const cv::Mat &image) {
    cv::namedWindow("main", cv::WINDOW_NORMAL);
    cv::imshow("main", image);


    int ESCAPE_KEY = 27;
    int key;
    do {
        key = cv::waitKey(0);
    } while ((key & 0xEFFFFF) != ESCAPE_KEY);
}

void convertMatTo2DArray(const cv::Mat &mat,
                         unsigned char imgData[IMG_HEIGHT][IMG_WIDTH]) {
    unsigned char *dataMat = mat.data;
    for (int j = 0; j < IMG_HEIGHT; j++) {
        for (int i = 0; i < IMG_WIDTH; i++) {
            imgData[j][i] = dataMat[j * IMG_WIDTH + i];
        }
    }
}

float **loadColumnPixelCountsFromFile(const string &filePath) {
    u_int16_t num_rotations, acc_width;
    std::ifstream fin("src/scripts/columnPixelCounts.dat", std::ios::binary);
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

void normalizeAccByNumPixels(unsigned int **acc, float **pixelCounts,
                             float **normalizedAcc) {
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        for (int col = 0; col < ACC_SIZE; col++) {
            float pixel_count = pixelCounts[rot][col];
            normalizedAcc[rot][col] = acc[rot][col] / pixel_count;
        }
    }
}

void ignoreColumnsWithTooFewPixels(float **slopes, float **pixelCounts,
                                   int pixelCountThreshold) {
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        for (int col = 0; col < ACC_SIZE; col++) {
            if (pixelCounts[rot][col] < pixelCountThreshold) {
                slopes[rot][col] = std::numeric_limits<float>::max();
            }
        }
    }
}


int main(int argc, char **argv) {
    RunParams params;
    int code = parseArgs(argc, argv, params);
    if (code != 0) {
        return code;
    }

    if (params.imagePath.empty()) {
        cout << "No input file. Use option --image." << endl;
        return 2;
    }
    if (params.averagingFilterSize <= 0) {
        cout << "Filter size is too small." << endl;
        return 2;
    }
    if (params.slopeThreshold <= 0) {
        cout << "Slope threshold cannot be zero or negative." << endl;
        return 2;
    }

    float **columnPixelCounts = loadColumnPixelCountsFromFile("src/scripts/columnPixelCounts.dat");

    cv::Mat img;
    readImage(params.imagePath, img);

    cv::Mat gray;
    convertBgrToGray(img, gray);
    //displayImage(gray);

    cv::Mat cleaned;
    removeExtremeIntensities(gray, cleaned);
    //displayImage(cleaned);

    unsigned char imgData[IMG_HEIGHT][IMG_WIDTH];
    convertMatTo2DArray(cleaned, imgData);
    //unsigned acc[NUM_ROTATIONS][ACC_SIZE];
    auto acc = allocate2DArray<unsigned>(NUM_ROTATIONS, ACC_SIZE);
    fill2DArray<unsigned>(acc, NUM_ROTATIONS, ACC_SIZE, 0);
    sumColumns(imgData, rotations, acc);

    // Divide by the number of nonzero pixels
    auto normalizedAcc = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    normalizeAccByNumPixels(acc, columnPixelCounts, normalizedAcc);

    auto average = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    fill2DArray<float>(average, NUM_ROTATIONS, ACC_SIZE, 0);
    convolveAverage(normalizedAcc, params.averagingFilterSize, average);

    auto peaks = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    extractPeaks(normalizedAcc, average, peaks);

    int sideDistance = params.averagingFilterSize / 2;
    auto slopes = allocate2DArray<float>(NUM_ROTATIONS, ACC_SIZE);
    extractSlopes(average, sideDistance, slopes);

    ignoreColumnsWithTooFewPixels(slopes, columnPixelCounts, params.minPixelsThreshold);
//    for (int i = 0; i < ACC_SIZE; i++) {
//        cout << i << ": " << columnPixelCounts[0][i] << ", " << normalizedAcc[0][i] << endl;
//    }

    auto selectedPeaks = allocate2DArray<float>(NUM_ROTATIONS, 2);
    selectPeaksUsingSlopes(peaks, slopes, params.slopeThreshold, selectedPeaks);

    float *bestPeak = selectedPeaks[0];
    float rotation;
    for (int i = 0; i < NUM_ROTATIONS; i++) {
        float angleRads = atan(rotations[i][1]/rotations[i][0]);
        float angleDegs = angleRads / M_PI * 180;

        if (selectedPeaks[i][1] > bestPeak[1]) {
            bestPeak = selectedPeaks[i];
            rotation = angleDegs;
        }
        cout << i << " (" << angleDegs << "): " << selectedPeaks[i][1] << " at " <<
             selectedPeaks[i][0] << endl;
    }
    cout << "Col: " << bestPeak[0] << ", metric: " << bestPeak[1] << ", rotation: " << rotation << "°" << endl;

    delete2DArray(columnPixelCounts, NUM_ROTATIONS);
    delete2DArray(acc, NUM_ROTATIONS);
    delete2DArray(normalizedAcc, NUM_ROTATIONS);
    delete2DArray(average, NUM_ROTATIONS);
    delete2DArray(peaks, NUM_ROTATIONS);
    delete2DArray(slopes, NUM_ROTATIONS);
    delete2DArray(selectedPeaks, NUM_ROTATIONS);
    return 0;
}
