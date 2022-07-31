
#ifndef ROTLINEDET_GPU_RUNPARAMS_H
#define ROTLINEDET_GPU_RUNPARAMS_H

#include <string>

class RunParams {
public:
    std::string pixelCountFilePath = "src/scripts/columnPixelCounts317.dat";
    int averagingFilterSize;
    float slopeThreshold;
    int minPixelsThreshold;
    bool verbose = false;
    int candidates = 1;
};
#endif //ROTLINEDET_GPU_RUNPARAMS_H
