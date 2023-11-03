#include "matrix.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mkl.h>
#include <mkl_service.h>
#include <mkl_spblas.h>
#include <omp.h>
#include <string>

int main(int argc, char ** argv) {
    // repeat test until 2 seconds cost
    // unit: ns
    auto measureTime = std::chrono::nanoseconds::rep(2'000'000'000);
    auto threadNum = 1;
    auto m = std::size_t(1024), n = std::size_t(1024), p = std::size_t(1024);

    // parse arguments
    for(auto index = std::size_t(1); index < argc; index++) {
        if(argv[index] == std::string("-M") || argv[index] == std::string("--measure-time")) {
            measureTime = std::stoll(argv[index + 1]) * 1'000'000'000;
            index++;
        }
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
        else {
            std::cout << "Sparse-dense matrix multiplication." << std::endl;
            std::cout << "Usage: SPMM [-M <measure-time>] [-t <threads>] [-m <dimension-1>] [-n <dimension-2>] [-p <dimension-3>]" << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "    -M, --measure-time  the least time to measure(unit: second)." << std::endl;
            std::cout << "    -t, --threads       the number of jobs(threads)." << std::endl;
            std::cout << "    -m, -n, -p          three dimensions of two matrix." << std::endl;
            return 0;
        }
    }
    std::cout << "Executing with arguments:   -M " << measureTime << " -t " << threadNum << " -m " << m << " -n " << n << " -p " << p << std::endl;

    omp_set_num_threads(threadNum);
    mkl_set_num_threads(threadNum);

    auto timeReport = std::ofstream();
    auto totalTime = std::chrono::nanoseconds::rep(0);
    auto repeatTimes = std::size_t(0);

    auto csr = CSRMatrix<double>();
    auto dense = DenseMatrix<double>();
    std::ifstream("testcases/sparse/sparse_" + std::to_string(m) + 'x' + std::to_string(n), std::ios::in | std::ios::binary) >> csr;
    std::ifstream("testcases/dense/dense_" + std::to_string(n) + 'x' + std::to_string(p), std::ios::in | std::ios::binary) >> dense;

    // my_res
    {
        auto my_res = DenseMatrix<double>();
        system("mkdir -p testcases/my_res");

        timeReport.open("testcases/my_res/time_" + std::to_string(m) + 'x' + std::to_string(n) + 'x' + std::to_string(p) + "_t" + std::to_string(threadNum), std::ios::out | std::ios::trunc);
        totalTime = 0;
        repeatTimes = 0;

        timeReport << "Time(ns):" << std::endl;
        while(totalTime <= measureTime) {
            // start timing
            auto start = std::chrono::high_resolution_clock::now();

            // multiplication
            my_res = csr * dense;

            // end timing
            auto end = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            totalTime += time;
            timeReport << "    " << time << std::endl;
            repeatTimes++;
        }
        timeReport << "Total time(ns):" << std::endl << "    " << totalTime << std::endl;
        timeReport << "Repeat times:" << std::endl << "    " << repeatTimes << std::endl;
        timeReport << "Average Time(ns):" << std::endl << "    " << totalTime / repeatTimes << std::endl;
        std::ofstream("testcases/my_res/res_" + std::to_string(m) + 'x' + std::to_string(n) + 'x' + std::to_string(p), std::ios::out | std::ios::binary) << my_res;
        timeReport.close();
    }

    // mkl_res
    {
        // extract CSR information
        auto linePointers = new long long[csr.line_pointers().size()];
        for(auto i = 0; i < csr.line_pointers().size(); i++) {
            linePointers[i] = csr.line_pointers()[i];
        }
        auto columeIndices = new long long[csr.column_indices().size()];
        for(auto i = 0; i < csr.column_indices().size(); i ++) {
            columeIndices[i] = csr.column_indices()[i];
        }

        // create MKL CSR matrix
        auto mkl_csr = sparse_matrix_t();
        mkl_sparse_d_create_csr(&mkl_csr, SPARSE_INDEX_BASE_ZERO, m, n, linePointers, linePointers + 1, columeIndices, csr.begin().base());
        auto mkl_csr_description = matrix_descr();
        mkl_csr_description.type = SPARSE_MATRIX_TYPE_GENERAL;
        auto mkl_dense = dense.matrix();
        auto mkl_res = new double[m * p];
        system("mkdir -p testcases/mkl_res");

        // MKL multiplication configs

        timeReport.open("testcases/mkl_res/time_" + std::to_string(m) + 'x' + std::to_string(n) + 'x' + std::to_string(p) + "_t" + std::to_string(threadNum), std::ios::out | std::ios::trunc);
        totalTime = 0;
        repeatTimes = 0;

        timeReport << "Time(ns):" << std::endl;
        while(totalTime <= measureTime) {
            // start timing
            auto start = std::chrono::high_resolution_clock::now();

            // multiplication
            mkl_sparse_d_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, mkl_csr, mkl_csr_description, SPARSE_LAYOUT_ROW_MAJOR, mkl_dense, p, m, 0.0, mkl_res, p);

            // end timing
            auto end = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            totalTime += time;
            timeReport << "    " << time << std::endl;
            repeatTimes++;
        }
        timeReport << "Total time(ns):" << std::endl << "    " << totalTime << std::endl;
        timeReport << "Repeat times:" << std::endl << "    " << repeatTimes << std::endl;
        timeReport << "Average Time(ns):" << std::endl
                   << "    " << totalTime / repeatTimes << std::endl;

        auto resultFile = std::ofstream("testcases/mkl_res/res_" + std::to_string(m) + 'x' + std::to_string(n) + 'x' + std::to_string(p), std::ios::out | std::ios::binary);

        auto bufferSize = sizeof(std::size_t) > sizeof(double) ? sizeof(std::size_t) : sizeof(double);
        alignas(std::max_align_t) char buffer[bufferSize];

        *(std::size_t *)buffer = m;
        resultFile.write(buffer, sizeof(std::size_t));
        *(std::size_t *)buffer = p;
        resultFile.write(buffer, sizeof(std::size_t));

        for(auto lineIndex = std::size_t(0); lineIndex < m; lineIndex++) {
            for(auto columnIndex = std::size_t(0); columnIndex < p; columnIndex++) {
                *(double *)buffer = mkl_res[lineIndex * p + columnIndex];
                resultFile.write(buffer, sizeof(double));
            }
        }

        timeReport.close();
    }

    // tvm_res
}