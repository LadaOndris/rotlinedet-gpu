#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <omp.h>
#include <openacc.h>
#include "../include/linedet.hpp"
#include <cassert>
#include <iostream>
#include <chrono>
#include "arrays.hpp"

using namespace std;
using namespace std::chrono;

void initialize_image(unsigned char img[IMG_HEIGHT][IMG_WIDTH]) {
	for (int row = 0; row < IMG_HEIGHT; row++) {
		for (int col = 0; col < IMG_WIDTH; col++) {
			img[row][col] = 1;
		}
	}
}

template <typename T>
int reduce_sum(T *arr, int size) {
	int sum = 0;
	for (int i = 0; i < size; i++) {
		sum += arr[i];
	}
	return sum;
}

int main_test() {
	cout << "Initializing arrays..." << endl;
	unsigned char img[IMG_HEIGHT][IMG_WIDTH];
	initialize_image(img);

	auto acc = allocate2DArray<unsigned>(NUM_ROTATIONS, ACC_SIZE);

	cout << "Performing column sum..." << endl;

	auto start = high_resolution_clock::now();
    sumColumns(img, rotations, acc);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " ms" << endl;

	cout << "Checking the result..." << endl;
	
	int image_sum = IMG_HEIGHT * IMG_WIDTH;
	for (int rot = 0; rot < NUM_ROTATIONS; rot++) {
		int acc_sum = reduce_sum<unsigned int>(acc[rot], ACC_SIZE);
		assert(acc_sum == image_sum);
	}

	cout << "Everything seems to be in check..." << endl;

	return 0;
}
