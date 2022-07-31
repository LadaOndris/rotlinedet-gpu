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
                 int minPixelsThreshold, float slopeThreshold, int candidates,
                 bool verbosely);
    ~LineDetector();

    std::vector<image_endpoints_t> processImage(std::vector<unsigned char> &imageVectorData);
private:
    int averagingFilterSize;
    int minPixelsThreshold;
    float slopeThreshold;
    int candidates;
    bool verbosely;

    int sideDistance;
    unsigned char imgData[IMG_HEIGHT][IMG_WIDTH];
    unsigned **acc;
    float **normalizedAcc;
    float **average;
    float **peaks;
    float **slopes;
    float **columnPixelCounts;

    static float **loadColumnPixelCountsFromFile(const std::string &filePath);

    std::vector<image_endpoints_t> determineImageEndpoints(
            std::vector<Peak> peaks) const;

    void moveBestPeaksToFront(std::vector<Peak> &peaks) const;

    void setImgData(std::vector<unsigned char> &vector);
};


#endif //ROTLINEDET_DETECTOR_H
