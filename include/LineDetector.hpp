//
// Created by lada on 7/25/22.
//

#ifndef ROTLINEDET_DETECTOR_H
#define ROTLINEDET_DETECTOR_H

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
#include "coordinates.hpp"
#include <sys/stat.h>
#include <string>
#include <vector>
#include "coordinates.hpp"

class LineDetector {
public:
    LineDetector(std::string &pixelCountFilePath, int averagingFilterSize,
                 int minPixelsThreshold, float slopeThreshold, bool verbosely);
    ~LineDetector();

    image_endpoints_t processImage(std::vector<unsigned char> &imageVectorData);
private:
    int averagingFilterSize;
    int minPixelsThreshold;
    float slopeThreshold;
    bool verbosely;

    int sideDistance;
//    cv::Mat img;
//    cv::Mat gray;
//    cv::Mat cleaned;
    unsigned char imgData[IMG_HEIGHT][IMG_WIDTH];
    unsigned **acc;
    float **normalizedAcc;
    float **average;
    float **peaks;
    float **slopes;
    float **selectedPeaks;
    float **columnPixelCounts;

//    static void convertMatTo2DArray(const cv::Mat &mat,
//                             unsigned char image[IMG_HEIGHT][IMG_WIDTH]);
    static float **loadColumnPixelCountsFromFile(const std::string &filePath);
    void getMaxPeak(float &rotation, float &column);

    void setImgData(std::vector<unsigned char> &vector);
};


#endif //ROTLINEDET_DETECTOR_H
