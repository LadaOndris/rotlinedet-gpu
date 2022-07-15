
#include "../include/linedet.hpp"
#include <iostream>
#include <openacc.h>

void readImage(const std::string &imagePath, cv::Mat &image) {
    cv::Mat img = cv::imread(imagePath);
    image = img;
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
