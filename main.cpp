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

using namespace std;
using namespace std::chrono;

float rotations[NUM_ROTATIONS][2] = {
	{ 0.9344164886263372, 0.35618229291084397 },
	{ 0.9312549088296838, 0.36436835041015486 },
	{ 0.928021615770611, 0.37252634895065384 },
	{ 0.9247168584357193, 0.38065566030833436 },
	{ 0.9213408913148668, 0.3887556584683067 },
	{ 0.9178939743815733, 0.3968257196730067 },
	{ 0.9143763730729988, 0.4048652224702291 },
	{ 0.9107883582695042, 0.4128735477609836 },
	{ 0.9071302062737913, 0.4208500788471697 },
	{ 0.9034021987896249, 0.42879420147906744 },
	{ 0.8996046229001404, 0.43670530390263884 },
	{ 0.8957377710457359, 0.4445827769066373 },
	{ 0.8918019410015525, 0.4524260138695203 },
	{ 0.8877974358545433, 0.4602344108061653 },
	{ 0.8837245639801331, 0.46800736641437984 },
	{ 0.8795836390184726, 0.4757442821212056 },
	{ 0.8753749798502843, 0.4834445621290142 },
	{ 0.8710989105723077, 0.49110761346138665 },
	{ 0.8667557604723404, 0.4987328460087775 },
	{ 0.8623458640038818, 0.5063196725739566 },
	{ 0.8578695607603769, 0.5138675089172287 },
	{ 0.8533271954490654, 0.521375773801423 },
	{ 0.8487191178644369, 0.5288438890366534 },
	{ 0.8440456828612944, 0.5362712795248419 },
	{ 0.8393072503274277, 0.5436573733040072 },
	{ 0.8345041851558992, 0.5510016015923082 },
	{ 0.829636857216945, 0.5583033988318452 },
	{ 0.8247056413294924, 0.565562202732211 },
	{ 0.8197109172322956, 0.5727774543137925 },
	{ 0.8146530695546934, 0.5799485979508148 },
	{ 0.80953248778699, 0.5870750814141296 },
	{ 0.8043495662504618, 0.5941563559137392 },
	{ 0.799104704066991, 0.6011918761410592 },
	{ 0.793798305128331, 0.6081811003109099 },
	{ 0.7884307780650044, 0.615123490203237 },
	{ 0.783002536214834, 0.6220185112045603 },
	{ 0.7775139975911141, 0.6288656323491412 },
	{ 0.77196558485042, 0.6356643263598714 },
	{ 0.7663577252600611, 0.6424140696888766 },
	{ 0.760690850665177, 0.6491143425578342 },
	{ 0.7549653974554837, 0.6557646289979993 },
	{ 0.7491818065316672, 0.662364416889938 },
	{ 0.7433405232714324, 0.6689131980029644 },
	{ 0.7374419974952046, 0.6754104680342782 },
	{ 0.7314866834314908, 0.6818557266477989 },
	{ 0.7254750396819006, 0.6882484775126965 },
	{ 0.7194075291858312, 0.6945882283416107 },
	{ 0.7132846191848166, 0.7008744909285622 },
	{ 0.7071067811865476, 0.7071067811865475 }
};

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

int main() {
	cout << "Initializing arrays..." << endl;

	int *rotated_cols = new int[NUM_ROTATIONS * IMG_HEIGHT * IMG_WIDTH];

	unsigned char img[IMG_HEIGHT][IMG_WIDTH];
	initialize_image(img);

	unsigned int acc[NUM_ROTATIONS][ACC_SIZE] = { 0 };

	cout << "Performing column sum..." << endl;

	auto start = high_resolution_clock::now();
	sum_columns(img, rotations, rotated_cols, acc);
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

	delete[] rotated_cols;

	return 0;
}
