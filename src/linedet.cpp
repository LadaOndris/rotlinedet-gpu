
#include "linedet.hpp"
#include <iostream>

void sum_columns(const unsigned char img[IMG_HEIGHT][IMG_WIDTH], 
		const float rotations[NUM_ROTATIONS][2], 
		unsigned int acc[NUM_ROTATIONS][ACC_SIZE]) {

	// Process all pixels in the image	
	for (int row = 0; row < IMG_HEIGHT; row++) {
		for (int col = 0; col < IMG_WIDTH; col++) {
		
			for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
				int col_shifted = col - HALF_IMG_WIDTH;
				int row_shifted = row - HALF_IMG_HEIGHT;
				float new_col_shifted = col_shifted * rotations[rot][0] + row_shifted * rotations[rot][1];
				int new_col = new_col_shifted + HALF_ACC_SIZE;
				acc[rot][new_col] += img[row][col];
			}
		}
	}
			
}
