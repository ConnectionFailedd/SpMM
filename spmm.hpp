#ifndef __SPMM_HPP__
#define __SPMM_HPP__

#include <cstddef>
#include <mkl_types.h>

void spmm_csr_dense(MKL_INT m, MKL_INT n, MKL_INT p,
                    const double * csrValues, const MKL_INT * csrColumnIndices, const MKL_INT * csrRowPointers,
                    const double * denseMatrix,
                    double * resultMatrix);

#endif