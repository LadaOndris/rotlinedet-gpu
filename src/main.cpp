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
            params.slopeThreshold = stoi(args[i + 1]);
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
    if (params.averagingFilterSize < 1) {
        cout << "Filter size is too small." << endl;
        return 2;
    }
    if (params.slopeThreshold < 1) {
        cout << "Slope threshold cannot be zero or negative." << endl;
        return 2;
    }

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
    auto acc = allocate2DArray(NUM_ROTATIONS, ACC_SIZE);
    sumColumns(imgData, rotations, acc);

    // Divide by the number of nonzero pixels - can be precomputed into coefficients and multiplied by it.

    auto average = allocate2DArray(NUM_ROTATIONS, ACC_SIZE);
    convolveAverage(acc, params.averagingFilterSize, average);

    auto peaks = allocate2DArray(NUM_ROTATIONS, ACC_SIZE);
    extractPeaks(acc, average, peaks);

    int sideDistance = params.averagingFilterSize / 2;
    auto slopes = allocate2DArray(NUM_ROTATIONS, ACC_SIZE);
    extractSlopes(average, sideDistance, slopes);

    auto selectedPeaks = allocate2DArray(NUM_ROTATIONS, 2);
    selectPeaksUsingSlopes(peaks, slopes, params.slopeThreshold, selectedPeaks);

    for (int i = 0; i < NUM_ROTATIONS; i++) {
        cout << i << ": " << selectedPeaks[i][1] << " at " << selectedPeaks[i][0] << endl;
    }

    delete2DArray(acc, NUM_ROTATIONS);
    delete2DArray(average, NUM_ROTATIONS);
    delete2DArray(peaks, NUM_ROTATIONS);
    delete2DArray(slopes, NUM_ROTATIONS);
    delete2DArray(selectedPeaks, NUM_ROTATIONS);
    return 0;
}