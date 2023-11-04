#include "spmm.hpp"
#include <cstddef>
#include <iostream>
#include <omp.h>
#include <ostream>

void spmm_csr_dense(MKL_INT m, MKL_INT n, MKL_INT p,
                    const MKL_INT * csrRowPointers, const MKL_INT * csrColumnIndices, const double * csrValues,
                    const double * denseMatrix,
                    double * resultMatrix,
                    double alpha, double beta) {
    if(beta < 1e-6 && beta > -1e-6) {
#pragma omp parallel for
        for(MKL_INT index = 0; index < n * p; index++) {
            resultMatrix[index] = 0;
        }
    }
    else {
#pragma omp parallel for
        for(MKL_INT index = 0; index < n * p; index++) {
            resultMatrix[index] *= beta;
        }
    }

    if(alpha - 1.0 < 1e-6 && 1.0 - alpha < 1e-6) {
#pragma omp parallel for schedule(dynamic, 4)
        for(size_t rowIndex = 0; rowIndex < m; rowIndex++) {
            for(size_t columnPointer = csrRowPointers[rowIndex]; columnPointer < csrRowPointers[rowIndex + 1]; columnPointer++) {
                size_t columnIndex = csrColumnIndices[columnPointer];
                double value = csrValues[columnPointer];
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
    else {
#pragma omp parallel for schedule(dynamic, 4)
        for(size_t rowIndex = 0; rowIndex < m; rowIndex++) {
            for(size_t columnPointer = csrRowPointers[rowIndex]; columnPointer < csrRowPointers[rowIndex + 1]; columnPointer++) {
                size_t columnIndex = csrColumnIndices[columnPointer];
                double value = csrValues[columnPointer];
                auto rhsIndex = columnIndex * p;
                auto rhsIndexEnd = (columnIndex + 1) * p;
                auto resIndex = rowIndex * p;
                while(rhsIndex < rhsIndexEnd) {
                    resultMatrix[resIndex] += alpha * denseMatrix[rhsIndex] * value;
                    rhsIndex ++;
                    resIndex ++;
                }
            }
        }
    }
}