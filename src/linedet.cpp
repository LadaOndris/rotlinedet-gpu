
#include "../include/linedet.hpp"
#include <iostream>
#include <openacc.h>

void readImage(const std::string &imagePath, cv::Mat &image) {
    cv::Mat img = cv::imread(imagePath);
    image = img;
}

void convert_bgr_to_gray(const cv::Mat &inImage, cv::Mat &outImage) {
    cv::cvtColor(inImage, outImage, cv::COLOR_BGR2GRAY);
}

void remove_extreme_intensities(const cv::Mat &inImage, cv::Mat &outImage) {
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

void convolve_average(unsigned int acc[NUM_ROTATIONS][ACC_SIZE], int filterSize,
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

// 440 ms
void sum_columns(const unsigned char img[IMG_HEIGHT][IMG_WIDTH],
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
