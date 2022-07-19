
#ifndef ROTLINEDET_GPU_RUNPARAMS_H
#define ROTLINEDET_GPU_RUNPARAMS_H

#include <string>

class RunParams {
public:
    std::string imagePath;
    int averagingFilterSize;
    int slopeThreshold;
};
#endif //ROTLINEDET_GPU_RUNPARAMS_H
