#include "linedet.hpp"
#include <iostream>
#include "RunParams.hpp"
#include "coordinates.hpp"
#include <sys/stat.h>
#include <LineDetector.hpp>
#include <vector>
#include <array>

using namespace std;
using namespace std::chrono;

inline bool exists_file(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

int checkParams(const RunParams &params) {
    if (params.averagingFilterSize <= 0) {
        cout << "Filter size is too small." << endl;
        return 2;
    }
    if (params.slopeThreshold <= 0) {
        cout << "Slope threshold cannot be zero or negative." << endl;
        return 2;
    }
    if (!exists_file(params.pixelCountFilePath)) {
        cout << "The pixel count file doesn't exist: " << params.pixelCountFilePath << endl;
        return 2;
    }
    return 0;
}

int parseArgs(int argc, char **argv, RunParams &params) {
    vector<string> args(argv + 1, argv + argc);

    for (std::vector<string>::size_type i = 0; i < args.size(); i += 2) {
        if (i + 1 == args.size()) {
            cout << "Expected parameter value" << endl;
            return 1;
        }
        if (args[i] == "--filterSize") {
            params.averagingFilterSize = stoi(args[i + 1]);
        } else if (args[i] == "--slopeThreshold") {
            params.slopeThreshold = stof(args[i + 1]);
        } else if (args[i] == "--minPixelsThreshold") {
            params.minPixelsThreshold = stoi(args[i + 1]);
        } else if (args[i] == "--pixelCountFile") {
            params.pixelCountFilePath = args[i + 1];
        } else if (args[i] == "--verbose") {
            params.verbose = args[i + 1] == "true";
        } else if (args[i] == "--candidates") {
            params.candidates = stoi(args[i + 1]);
        } else {
            cout << "Unknown option: " << args[i] << endl;
            return 2;
        }
    }
    return checkParams(params);
}

void printImageEndpoints(const std::vector<image_endpoints_t> &endpointsArray) {
    for (auto endpoints : endpointsArray) {
        cout << endpoints.x1 << "," << endpoints.y1 << "," << endpoints.x2 << "," << endpoints.y2 << endl;
    }
}


void readImageDataFromSTDIN(std::vector<unsigned char> &input) {

    std::freopen(nullptr, "rb", stdin);

    if (std::ferror(stdin))
        throw std::runtime_error(std::strerror(errno));

    std::size_t len;
    std::array<unsigned char, RGB_NUM_ELEMS> buf;

    while ((len = std::fread(buf.data(), sizeof(buf[0]), buf.size(), stdin)) > 0) {
        if (std::ferror(stdin) && !std::feof(stdin))
            throw std::runtime_error(std::strerror(errno));

        input.insert(input.end(), buf.data(), buf.data() + len);
    }
}


void rgbToGray(const std::vector<unsigned char> &rgb,
               std::vector<unsigned char> &gray) {
    for (int i = 0; i + 2 < rgb.size(); i += 3) {
        auto value = 0.2125 * rgb[i] +
                     0.7154 * rgb[i + 1] +
                     0.0721 * rgb[i + 2];
        gray.push_back(value);
    }
}

/**
 * Reads image data from stdin.
 * The image can be grayscale or RGB.
 * If the image is RGB, it is converted to grayscale.
 *
 * @param imageData Single channel image pixel values
 */
std::vector<unsigned char> loadImageData() {
    std::vector<unsigned char> input;
    input.reserve(RGB_NUM_ELEMS);
    readImageDataFromSTDIN(input);
    if (input.size() != RGB_NUM_ELEMS && input.size() != GRAY_NUM_ELEMS) {
        throw std::runtime_error("Unexpected number of elements from stdin: " + to_string(input.size()));
    }

    if (input.size() == RGB_NUM_ELEMS) {
        std::vector<unsigned char> grayImageData;
        grayImageData.reserve(GRAY_NUM_ELEMS);
        rgbToGray(input, grayImageData);
        return grayImageData;
    }
    return input;
}


int main(int argc, char **argv) {
    RunParams params;
    int code = parseArgs(argc, argv, params);
    if (code != 0) {
        return code;
    }

    auto lineDetector = LineDetector(params.pixelCountFilePath, params.averagingFilterSize,
                                     params.minPixelsThreshold, params.slopeThreshold,
                                     params.candidates, params.verbose);
    std::vector<unsigned char> imageData = loadImageData();
    auto imageEndpoints = lineDetector.processImage(imageData);
    printImageEndpoints(imageEndpoints);

    return 0;
}
