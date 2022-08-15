
#ifndef LINEDET_HPP
#define LINEDET_HPP

#include <cmath>
#include <string>
#include <vector>
#include <cmath>

#ifdef HD

#define ACC_SIZE 4406
#define HALF_ACC_SIZE 2203
#define IMG_WIDTH 3840
#define IMG_HEIGHT 2160
#define HALF_IMG_WIDTH 1920
#define HALF_IMG_HEIGHT 1080

#else

#define ACC_SIZE 2204
#define HALF_ACC_SIZE 1102
#define IMG_WIDTH 1920
#define IMG_HEIGHT 1080
#define HALF_IMG_WIDTH 960
#define HALF_IMG_HEIGHT 540

#endif

#define RGB_NUM_ELEMS (IMG_HEIGHT * IMG_WIDTH * 3)
#define GRAY_NUM_ELEMS (IMG_HEIGHT * IMG_WIDTH)
#define NUM_ROTATIONS 281

extern float rotations[NUM_ROTATIONS][2];

struct Peak {
    unsigned rotation; // rotation index into rotations array
    unsigned column;
    float value;

    Peak(unsigned rotation, unsigned column, float peakValue)
    : rotation(rotation), column(column), value(peakValue) {}

    /**
     * @return Returns angle of rotation in radians.
     */
    double getRotationAngle() const {
        return std::atan(rotations[rotation][1] / rotations[rotation][0]);
    }
};

void removeExtremeIntensities(unsigned char inImage[IMG_HEIGHT][IMG_WIDTH],
                              unsigned char outImage[IMG_HEIGHT][IMG_WIDTH]);

void sumColumns(const unsigned char img[IMG_HEIGHT][IMG_WIDTH],
                const float rotations[NUM_ROTATIONS][2],
                unsigned int **acc);

void convolveAverage(float **acc, int filterSize,
                     float **result);

void extractPeaks(float **acc,
                  float **average,
                  float**peaks);

void extractSlopes(float **array,
                   int sideDistance,
                   float **slopes);

std::vector<Peak> selectPeaksUsingSlopes(
        float **peaks, float **slopes, float slopeThreshold);

void normalizeAccByNumPixels(unsigned int **acc, float **pixelCounts,
                             float **normalizedAcc);

void ignoreColumnsWithTooFewPixels(float **slopes, float **pixelCounts,
                                   int pixelCountThreshold);

#endif
