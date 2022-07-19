
#ifndef LINEDET_HPP
#define LINEDET_HPP

#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

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

#define NUM_ROTATIONS 180

void readImage(const std::string &imagePath, cv::Mat &image);

void convertBgrToGray(const cv::Mat &inImage, cv::Mat &outImage);

void removeExtremeIntensities(const cv::Mat &inImage, cv::Mat &outImage);

void sumColumns(const unsigned char img[IMG_HEIGHT][IMG_WIDTH],
                const float rotations[NUM_ROTATIONS][2],
                unsigned int acc[NUM_ROTATIONS][ACC_SIZE]);

void convolveAverage(unsigned int acc[NUM_ROTATIONS][ACC_SIZE], int filterSize,
                     unsigned int result[NUM_ROTATIONS][ACC_SIZE]);

void extractPeaks(unsigned int acc[NUM_ROTATIONS][ACC_SIZE],
                  unsigned int average[NUM_ROTATIONS][ACC_SIZE],
                  unsigned int peaks[NUM_ROTATIONS][ACC_SIZE]);

void extractSlopes(unsigned int array[NUM_ROTATIONS][ACC_SIZE],
                   int sideDistance,
                   unsigned int slopes[NUM_ROTATIONS][ACC_SIZE]);

void selectPeaksUsingSlopes(unsigned int peaks[NUM_ROTATIONS][ACC_SIZE],
                            unsigned int slopes[NUM_ROTATIONS][ACC_SIZE],
                            unsigned slopeThreshold,
                            unsigned int selectedPeaks[NUM_ROTATIONS][2]);

#endif
