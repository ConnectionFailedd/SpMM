#include "spmm.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mkl.h>
#include <mkl_spblas.h>
#include <mkl_types.h>
#include <omp.h>
#include <ostream>
#include <sstream>
#include <string>

int main(int argc, char ** argv) {
    // arguments
    size_t threadNum = 1;
    MKL_INT m = 1024, n = 1024, p = 1024;
    double density = 0.1;

    // parse arguments
    for(auto index = std::size_t(1); index < argc; index++) {
        if(argv[index] == std::string("-t") || argv[index] == std::string("--threads")) {
            threadNum = std::stoi(argv[index + 1]);
            index++;
        }
        else if(argv[index] == std::string("-m")) {
            m = std::stoul(argv[index + 1]);
            index++;
        }
        else if(argv[index] == std::string("-n")) {
            n = std::stoul(argv[index + 1]);
            index++;
        }
        else if(argv[index] == std::string("-p")) {
            p = std::stoul(argv[index + 1]);
            index++;
        }
        else if(argv[index] == std::string("-d") || argv[index] == std::string("--density")) {
            density = std::stod(argv[index + 1]);
            index++;
        }
        else {
            std::cout << "Sparse-dense matrix multiplication." << std::endl;
            std::cout << "Usage: SPMM [-t <threads>] [-m <dimension-1>] [-n <dimension-2>] [-p <dimension-3>]" << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "    -t, --threads       the number of jobs(threads)." << std::endl;
            std::cout << "    -m, -n, -p          three dimensions of two matrix." << std::endl;
            return 0;
        }
    }
    std::cout << "Executing with arguments: -t " << threadNum << " -m " << m << " -n " << n << " -p " << p << " -d " << density << std::endl;

    // set threads
    omp_set_num_threads(threadNum);
    mkl_set_num_threads(threadNum);

    // load sparse matrix
    auto sparseFileName = (std::ostringstream() << "testcases/csr_" << m << "x" << n << "_" <<  density).str();
    auto sparseFile = std::ifstream(sparseFileName);
    if(!sparseFile.is_open()) {
        std::cout << "File " << sparseFileName << " not found!" << std::endl;
        return 1;
    }
    MKL_INT nnz;
    sparseFile >> m >> n >> nnz;
    MKL_INT * cooRows = new MKL_INT[nnz];
    MKL_INT * cooColumns = new MKL_INT[nnz];
    double * cooValues = new double[nnz];
    for(MKL_INT count = 0; count < nnz; count++) {
        sparseFile >> cooRows[count] >> cooColumns[count] >> cooValues[count];
    }

    // load as COO format, then transform to CSR format
    sparse_matrix_t cooMatrix = nullptr;
    mkl_sparse_d_create_coo(&cooMatrix, SPARSE_INDEX_BASE_ZERO, m, n, nnz, cooRows, cooColumns, cooValues);
    sparse_matrix_t csrMatrix = nullptr;
    mkl_sparse_convert_csr(cooMatrix, SPARSE_OPERATION_NON_TRANSPOSE, &csrMatrix);

    // extract data from MKL CSR matrix
    MKL_INT * csrRowStartPointers = nullptr;
    MKL_INT * csrRowEndPointers = nullptr;
    MKL_INT * csrColumnIndices = nullptr;
    double * csrValues = nullptr;
    sparse_index_base_t indexing;
    MKL_INT rows, columns;
    mkl_sparse_d_export_csr(csrMatrix, &indexing, &rows, &columns, &csrRowStartPointers, &csrRowEndPointers, &csrColumnIndices, &csrValues);

    // load dense matrix
    auto denseFileName = (std::ostringstream() << "testcases/dense_" << n << "x" << p << "_" << density).str();
    auto denseFile = std::ifstream(denseFileName);
    if(!denseFile.is_open()) {
        std::cout << "File " << denseFileName << " not found!" << std::endl;
        return 1;
    }
    denseFile >> n >> p;
    double * denseMatrix = new double[n * p];
    for(MKL_INT count = 0; count < n * p; count++) {
        denseFile >> denseMatrix[count];
    }

    // result matrix
    double * resultMatrix = new double[m * p]();

    system("mkdir -p results");
    system("mkdir -p time_reports");

    // my result calculate
    {
        auto totalTime = size_t(0);
        auto repeatTimes = size_t(0);
        auto timeReportFileName = (std::ostringstream() << "time_reports/my_" << threadNum << "_" << m << "x" << n << "x" << p << "_" << density).str();
        auto timeReportFile = std::ofstream(timeReportFileName);
        while(totalTime < 60000000000) {
            // calculate & record time
            auto start = std::chrono::high_resolution_clock::now();
            spmm_csr_dense(m, n, p, csrRowStartPointers, csrColumnIndices, csrValues, denseMatrix, resultMatrix, 1, 0);
            auto end = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            totalTime += time;
            repeatTimes++;
        }
        // calculate average time
        timeReportFile << "Total time: " << totalTime << std::endl;
        timeReportFile << "Repeat times: " << repeatTimes << std::endl;
        timeReportFile << "Average time(ns): " << totalTime / repeatTimes << std::endl;
        // output result
        auto outputFileName = (std::ostringstream() << "results/my_" << threadNum << "_" << m << "x" << n << "x" << p << "_" << density).str();
        auto outputFile = std::ofstream(outputFileName);
        outputFile << m << " " << p << std::endl;
        for(MKL_INT rowIndex = 0; rowIndex < m; rowIndex++) {
            for(MKL_INT columnIndex = 0; columnIndex < p; columnIndex++) {
                outputFile << resultMatrix[rowIndex * p + columnIndex] << " ";
            }
            outputFile << std::endl;
        }
    }

    // MKL result calculate
    {
        auto mkl_csr_description = matrix_descr();
        mkl_csr_description.type = SPARSE_MATRIX_TYPE_GENERAL;
        auto totalTime = size_t(0);
        auto repeatTimes = size_t(0);
        auto timeReportFileName = (std::ostringstream() << "time_reports/mkl_" << threadNum << "_" << m << "x" << n << "x" << p << "_" << density).str();
        auto timeReportFile = std::ofstream(timeReportFileName);
        while(totalTime < 60000000000) {
            // calculate & record time
            auto start = std::chrono::high_resolution_clock::now();
            mkl_sparse_d_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrMatrix, mkl_csr_description, SPARSE_LAYOUT_ROW_MAJOR, denseMatrix, p, m, 0.0, resultMatrix, p);
            auto end = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            totalTime += time;
            repeatTimes++;
        }
        // calculate average time
        timeReportFile << "Total time: " << totalTime << std::endl;
        timeReportFile << "Repeat times: " << repeatTimes << std::endl;
        timeReportFile << "Average time(ns): " << totalTime / repeatTimes << std::endl;
        // output result
        auto outputFileName = (std::ostringstream() << "results/mkl_" << threadNum << "_" << m << "x" << n << "x" << p << "_" << density).str();
        auto outputFile = std::ofstream(outputFileName);
        outputFile << m << " " << p << std::endl;
        for(MKL_INT rowIndex = 0; rowIndex < m; rowIndex++) {
            for(MKL_INT columnIndex = 0; columnIndex < p; columnIndex++) {
                outputFile << resultMatrix[rowIndex * p + columnIndex] << " ";
            }
            outputFile << std::endl;
        }
    }

    // free resources
    delete[] cooRows;
    delete[] cooColumns;
    delete[] cooValues;
    delete[] denseMatrix;
    delete[] resultMatrix;
    mkl_sparse_destroy(csrMatrix);
    mkl_sparse_destroy(cooMatrix);
}