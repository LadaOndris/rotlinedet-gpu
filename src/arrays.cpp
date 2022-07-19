
#include "arrays.hpp"

unsigned **allocate2DArray(int sizeDim1, int sizeDim2) {
    auto array = new unsigned *[sizeDim1];
    for (int i = 0; i < sizeDim1; ++i) {
        array[i] = new unsigned[sizeDim2];
    }
    return array;
}

void delete2DArray(unsigned **array, int sizeDim1) {
    for (int i = 0; i < sizeDim1; ++i) {
        delete[] array[i];
    }
    delete[] array;
}