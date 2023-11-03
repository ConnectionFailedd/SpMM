#include "spmm.hpp"
#include <cstddef>
#include <iostream>
#include <omp.h>

void spmm_csr_dense(MKL_INT m, MKL_INT n, MKL_INT p,
                    const MKL_INT * csrRowPointers, const MKL_INT * csrColumnIndices, const double * csrValues,
                    const double * denseMatrix,
                    double * resultMatrix) {
    for(size_t rowIndex = 0; rowIndex < m; rowIndex++) {
        for(size_t columnPointer = csrRowPointers[rowIndex]; columnPointer < csrRowPointers[rowIndex + 1]; columnPointer++) {
            size_t columnIndex = csrColumnIndices[columnPointer];
            double value = csrValues[columnPointer];
            // std::cout << "(" << rowIndex << ", " << columnIndex << ")" << value;
            auto rhsIndex = columnIndex * p;
            auto rhsIndexEnd = (columnIndex + 1) * p;
            auto resIndex = rowIndex * p;
            while(rhsIndex < rhsIndexEnd) {
                resultMatrix[resIndex] += denseMatrix[rhsIndex] * value;
                rhsIndex ++;
                resIndex ++;
            }
        }
    }
}