#include "spmm.hpp"
#include <cstddef>
#include <iostream>
#include <omp.h>

void spmm_csr_dense(MKL_INT m, MKL_INT n, MKL_INT p,
                    const double * csrValues, const MKL_INT * csrColumnIndices, const MKL_INT * csrRowPointers,
                    const double * denseMatrix,
                    double * resultMatrix) {
    for(size_t rowIndex = 0; rowIndex < m; rowIndex++) {
        for(size_t columnPointer = csrRowPointers[rowIndex]; columnPointer < csrRowPointers[rowIndex + 1]; columnPointer++) {
            size_t columnIndex = csrColumnIndices[columnPointer];
            double value = csrValues[columnPointer];
            std::cout << "(" << rowIndex << ", " << columnIndex << ")" << value;
        }
    }
    return;
}