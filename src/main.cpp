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
        } else {
            cout << "Unknown option: " << args[i] << endl;
            return 2;
        }
    }
    return checkParams(params);
}

//void displayImage(const cv::Mat &image) {
//    cv::namedWindow("main", cv::WINDOW_NORMAL);
//    cv::imshow("main", image);
//
//
//    int ESCAPE_KEY = 27;
//    int key;
//    do {
//        key = cv::waitKey(0);
//    } while ((key & 0xEFFFFF) != ESCAPE_KEY);
//}

void printImageEndpoints(image_endpoints_t endpoints) {
    cout << endpoints.x1 << "," << endpoints.y1 << "," << endpoints.x2 << "," << endpoints.y2 << endl;
}


void readImageData(std::vector<unsigned char> &input) {

    std::freopen(nullptr, "rb", stdin);

    if (std::ferror(stdin))
        throw std::runtime_error(std::strerror(errno));

#define NUM_ELEMS (IMG_HEIGHT * IMG_WIDTH * 3)
    std::size_t len;
    std::array<unsigned char, NUM_ELEMS> buf;

    while ((len = std::fread(buf.data(), sizeof(buf[0]), buf.size(), stdin)) > 0) {
        if (std::ferror(stdin) && !std::feof(stdin))
            throw std::runtime_error(std::strerror(errno));

        input.insert(input.end(), buf.data(), buf.data() + len); // append to vector
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


int main(int argc, char **argv) {
    RunParams params;
    int code = parseArgs(argc, argv, params);
    if (code != 0) {
        return code;
    }


    auto lineDetector = LineDetector(params.pixelCountFilePath, params.averagingFilterSize,
                                     params.minPixelsThreshold, params.slopeThreshold,
                                     params.verbose);
    std::vector<unsigned char> input;
    input.reserve(NUM_ELEMS);
    readImageData(input);
    if (input.size() != NUM_ELEMS) {
        throw std::runtime_error("Unexpected number of elements from stdin: " + to_string(input.size()));
    }

    std::vector<unsigned char> grayImageData;
    grayImageData.reserve(IMG_HEIGHT * IMG_WIDTH);
    rgbToGray(input, grayImageData);

    auto imageEndpoints = lineDetector.processImage(grayImageData);
    printImageEndpoints(imageEndpoints);

    return 0;
}
