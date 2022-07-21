//
// Created by lada on 7/19/22.
//

#ifndef ROTLINEDET_ARRAYS_HPP
#define ROTLINEDET_ARRAYS_HPP

template<typename T>
T **allocate2DArray(int sizeDim1, int sizeDim2) {
    auto array = new T *[sizeDim1];
    for (int i = 0; i < sizeDim1; ++i) {
        array[i] = new T[sizeDim2];
    }
    return array;
}

template<typename T>
void delete2DArray(T **array, int sizeDim1) {
    for (int i = 0; i < sizeDim1; ++i) {
        delete[] array[i];
    }
    delete[] array;
}

template<typename T>
void fill2DArray(T **array, int sizeDim1, int sizeDim2, T fillValue) {
    for (int i = 0; i < sizeDim1; ++i) {
        for (int j = 0; j < sizeDim2; ++j) {
            array[i][j] = fillValue;
        }
    }
}

#endif //ROTLINEDET_ARRAYS_HPP
