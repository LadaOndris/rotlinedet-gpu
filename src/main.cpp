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
#include "RunParams.hpp"

using namespace std;
using namespace std::chrono;

int parseArgs(int argc, char **argv, RunParams &params) {
    vector<string> args(argv + 1, argv + argc);

    for (std::vector<string>::size_type i = 0; i < args.size(); i += 2) {
        if (i + 1 == args.size()) {
            cout << "Expected parameter value" << endl;
            return 1;
        }
        auto nextArg = args[i + 1].c_str();

        if (args[i] == "--image") {
            params.imagePath = args[i + 1];
        }else {
            cout << "Unknown option: " << args[i] << endl;
            return 2;
        }
    }
    return 0;
}

void displayImage(const cv::Mat &image) {
    cv::namedWindow("main", cv::WINDOW_NORMAL);
    cv::imshow("main", image);

    auto key = cv::waitKey();
    int ESCAPE_KEY = 27;
    while ((key & 0xEFFFFF) != ESCAPE_KEY);
}

int main(int argc, char **argv) {
    RunParams params;
    int code = parseArgs(argc, argv, params);
    if (code != 0) {
        return code;
    }

    cv::Mat img;
    readImage(params.imagePath, img);
    displayImage(img);

    return 0;
}