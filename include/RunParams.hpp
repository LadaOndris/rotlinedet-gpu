
#ifndef ROTLINEDET_GPU_RUNPARAMS_H
#define ROTLINEDET_GPU_RUNPARAMS_H

#include <string>

class RunParams {
public:
    std::string imagePath;
    std::string pixelCountFilePath = "src/scripts/columnPixelCounts.dat";
    int averagingFilterSize;
    float slopeThreshold;
    int minPixelsThreshold;
    bool verbose = false;
};
#endif //ROTLINEDET_GPU_RUNPARAMS_H
