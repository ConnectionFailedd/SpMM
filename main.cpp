#include "spmm.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <mkl_spblas.h>
#include <mkl_types.h>

int main() {
    auto sparseFile = std::ifstream("sparse_matrix.txt");
    MKL_INT m, n, p, nnz;
    sparseFile >> m >> n;
    sparseFile >> nnz;
    MKL_INT cooRows[nnz];
    MKL_INT cooColumns[nnz];
    double cooValues[nnz];
    for(MKL_INT count = 0; count < nnz; count++) {
        sparseFile >> cooRows[count] >> cooColumns[count] >> cooValues[count];
    }
    std::cout << m << n << nnz << std::endl;

    sparse_matrix_t coo_matrix = nullptr;
    mkl_sparse_d_create_coo(&coo_matrix, SPARSE_INDEX_BASE_ZERO, m, n, nnz, cooRows, cooColumns, cooValues);

    sparse_matrix_t csr_matrix = nullptr;
    mkl_sparse_convert_csr(coo_matrix, SPARSE_OPERATION_NON_TRANSPOSE, &csr_matrix);

    // MKL_INT * csrRowPointers = new MKL_INT[m + 1]();
    // MKL_INT * csrRowPointers1 = new MKL_INT[m + 1]();
    // MKL_INT * csrColumnIndices = new MKL_INT[nnz]();
    MKL_INT csrRowStartPointers[m + 1];
    MKL_INT csrRowEndPointers[m];
    MKL_INT csrColumnIndices[nnz];
    double csrValues[nnz];
    // double * csrValues = new double[nnz]();
    sparse_index_base_t indexing;
    MKL_INT rows, columns;
    mkl_sparse_d_export_csr(csr_matrix, &indexing, &rows, &columns, &csrRowStartPointers, &csrRowEndPointers, &csrColumnIndices, &csrValues);

    auto denseFile = std::ifstream("dense_matrix.txt");
    MKL_INT p;
    denseFile >> n >> p;
    if(n != columns) {
        std::cout << "Invalid matrices with size (" << m << ", " << columns << ") and (" << n << ", " << p << ")." << std::endl;
        return 1;
    }
    double denseMatrix[n * p];
    for(MKL_INT count = 0; count < n * p; count ++) {
        denseFile >> denseMatrix[count];
    }

    double resultMatrix
    spmm_csr_dense(m, n, p, csrRowPointers, csrColumnIndices, csrValues, denseMatrix, resultMatrix);

    auto outputFile = std::ofstream("my_res");
    outputFile << m << " " << p << std::endl;
    for(MPI_INT rowIndex = 0; rowIndex < m; rowIndex ++) {
        for(MPI_INT columnIndex = 0; columnIndex < p; columnIndex ++) {
            outputFile << denseMatrix[rowIndex * p + columnIndex] << " ";
        }
        outputFile << std::endl;
    }
}