
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

void convolveAverage(unsigned int **acc, int filterSize,
                     unsigned int **result) {
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

void extractPeaks(unsigned int **acc,
                  unsigned int **average,
                  unsigned int **peaks) {
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
void extractSlopes(unsigned int **array,
                   int sideDistance,
                   unsigned int **slopes) {
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

void selectPeaksUsingSlopes(unsigned int **peaks,
                            unsigned int **slopes,
                            unsigned slopeThreshold,
                            unsigned int **selectedPeaks) {
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
                unsigned int **acc) {

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
