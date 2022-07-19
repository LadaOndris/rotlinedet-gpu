
#include "../include/linedet.hpp"
#include <iostream>
#include <openacc.h>

void readImage(const std::string &imagePath, cv::Mat &image) {
    cv::Mat img = cv::imread(imagePath);
    image = img;
}

void convertBgrToGray(const cv::Mat &inImage, cv::Mat &outImage) {
    cv::cvtColor(inImage, outImage, cv::COLOR_BGR2GRAY);
}

void removeExtremeIntensities(const cv::Mat &inImage, cv::Mat &outImage) {
    outImage = inImage;
//
//    cv::Mat reshaped = inImage.reshape(1, 1);
//    auto begin = reshaped.begin<cv::uint8_t>();
//    auto end = reshaped.end<cv::uint8_t>();
//    auto center = begin + reshaped.total() / 2;
//    std::nth_element(begin, center, end);
//    std::cout << "\nThe median is " << reshaped.total() << '\n';
//    std::cout << "\nThe median is " << reshaped.at<cv::uint8_t>(reshaped.total() / 2) << '\n';
}

void convolveAverage(unsigned int acc[NUM_ROTATIONS][ACC_SIZE], int filterSize,
                     unsigned int result[NUM_ROTATIONS][ACC_SIZE]) {
    // Convolve each image rotation
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        // Do the 1D convolution itself
        int halfFilterSize = filterSize / 2;
        for (int i = halfFilterSize; i < ACC_SIZE - halfFilterSize; i++) {
            // Sum values
            int firstConvIndex = i - halfFilterSize;
            for (int h = firstConvIndex; h < firstConvIndex + filterSize; h++) {
                result[rot][i] += acc[rot][h];
            }
            // Compute average by dividing by the number of values
            result[rot][i] /= filterSize;
        }
    }
}

void extractPeaks(unsigned int acc[NUM_ROTATIONS][ACC_SIZE],
                  unsigned int average[NUM_ROTATIONS][ACC_SIZE],
                  unsigned int peaks[NUM_ROTATIONS][ACC_SIZE]) {
    // diff = acc - average
    // peaks = max(0, diff)
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        for (int col = 0; col < ACC_SIZE; col++) {
            unsigned val = std::max(acc[rot][col], average[rot][col]);
            peaks[rot][col] = val - average[rot][col];
        }
    }
}

/**
 *
 * @param array
 *  The array on which the slopes are computed.
 * @param sideDistance
 *  Determines the distance between the values for which the
 *  slope is computed. It measures the distance from the center position to
 *  each side. E.g., windowSize = 2 indicates that
 *  the slope for index 2 will be computed between the 0th and the 4nd value.
 *  For index 5 it will be the 3rd and the 7th.
 */
void extractSlopes(unsigned int array[NUM_ROTATIONS][ACC_SIZE],
                   int sideDistance,
                   unsigned int slopes[NUM_ROTATIONS][ACC_SIZE]) {
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        for (int col = 0; col < sideDistance; col++) {
            slopes[rot][col] = std::numeric_limits<unsigned>::max();
        }
        for (int col = sideDistance; col < ACC_SIZE - sideDistance; col++) {
            unsigned left = array[rot][col - sideDistance];
            unsigned right = array[rot][col + sideDistance];
            unsigned bigger = std::max(left, right);
            unsigned smaller = std::min(left, right);
            slopes[rot][col] = bigger - smaller;
        }
        for (int col = ACC_SIZE - sideDistance; col < ACC_SIZE; col++) {
            slopes[rot][col] = std::numeric_limits<unsigned>::max();
        }
    }
}

void selectPeaksUsingSlopes(unsigned int peaks[NUM_ROTATIONS][ACC_SIZE],
                            unsigned int slopes[NUM_ROTATIONS][ACC_SIZE],
                            unsigned slopeThreshold,
                            unsigned int selectedPeaks[NUM_ROTATIONS][2]) {
    // Selects peak with the highest peak value above certain slope threshold
    // for each rotation
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        selectedPeaks[rot][0] = 0; // col
        selectedPeaks[rot][1] = 0; // peakValue

        for (int col = 0; col < ACC_SIZE; col++) {
            if (slopes[rot][col] < slopeThreshold) {
                if (selectedPeaks[rot][1] < peaks[rot][col]) {
                    selectedPeaks[rot][0] = col;
                    selectedPeaks[rot][1] = peaks[rot][col];
                }
            }
        }
    }
}

// 440 ms
void sumColumns(const unsigned char img[IMG_HEIGHT][IMG_WIDTH],
                const float rotations[NUM_ROTATIONS][2],
                unsigned int acc[NUM_ROTATIONS][ACC_SIZE]) {

#pragma acc data copyin(img[:IMG_HEIGHT][:IMG_WIDTH])
    for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
        auto acc_rot = acc[rot];
        auto rotation_col = rotations[rot][0];
        auto rotation_row = rotations[rot][1];
        {
#pragma acc parallel loop copy(acc_rot[:ACC_SIZE]) vector_length(512)
            for (int row = 0; row < IMG_HEIGHT; row++) {
#pragma acc loop
                for (int col = 0; col < IMG_WIDTH; col++) {

                    int col_shifted = col - HALF_IMG_WIDTH;
                    int row_shifted = row - HALF_IMG_HEIGHT;
                    float new_col_shifted = col_shifted * rotation_col + row_shifted * rotation_row;
                    int new_col = new_col_shifted + HALF_ACC_SIZE;
#pragma acc atomic update
                    acc_rot[new_col] += img[row][col];
                }
            }
        }
    }
}
