
#ifndef LINEDET_HPP
#define LINEDET_HPP

#define ACC_SIZE 2204
#define HALF_ACC_SIZE 1102
#define IMG_WIDTH 1920
#define IMG_HEIGHT 1080
#define HALF_IMG_WIDTH 960
#define HALF_IMG_HEIGHT 540

#define NUM_ROTATIONS 180

void sum_columns(const unsigned char img[IMG_HEIGHT][IMG_WIDTH], 
		const float rotations[NUM_ROTATIONS][2], 
		unsigned int acc[NUM_ROTATIONS][ACC_SIZE]);

#endif
