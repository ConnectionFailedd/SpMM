#include "matrix.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <string>

int main(int argc, char ** argv) {
    auto testcaseNum = std::size_t(128);
    auto threadsNum = 1;

    // parse arguments
    if(argc == 1) {
        std::cout << "Executing with default arguments:   -N " << testcaseNum << " -t " << threadsNum << std::endl;
    }
    for(auto index = std::size_t(1); index < argc; index++) {
        if(argv[index] == std::string("-N") || argv[index] == std::string("--number")) {
            testcaseNum = std::stoul(argv[index + 1]);
            index++;
        }
        if(argv[index] == std::string("-t") || argv[index] == std::string("--threads")) {
            threadsNum = std::stoi(argv[index + 1]);
            index++;
        }
        else {
            std::cout << "Sparse-dense matrix multiplication." << std::endl;
            std::cout << "Usage: SPMM [-N <testcase-number>]" << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "    -N, --number        the number of testcases for testing." << std::endl;
            std::cout << "    -t, --threads       the number of threads." << std::endl;
        }
    }

    omp_set_num_threads(threadsNum);

    // my_res
    auto csr = CSRMatrix<double>();
    auto dense = DenseMatrix<double>();
    auto res = DenseMatrix<double>();

    system("mkdir -p testcases/my_res");
    auto timeRecorder = std::ofstream("testcases/my_res/_time" + std::to_string(threadsNum), std::ios::out | std::ios::trunc);
    for(auto index = 0; index < testcaseNum; index++) {
        std::ifstream("testcases/sparse/sparse" + std::to_string(index), std::ios::in | std::ios::binary) >> csr;
        std::ifstream("testcases/dense/dense" + std::to_string(index), std::ios::in | std::ios::binary) >> dense;

        // start timing
        auto start = std::chrono::high_resolution_clock::now();

        // multiplication
        res = csr * dense;

        // end timing
        auto end = std::chrono::high_resolution_clock::now();
        timeRecorder << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << std::endl;

        std::ofstream("testcases/my_res/res" + std::to_string(index), std::ios::out | std::ios::binary) << res;
    }
    timeRecorder.close();

    // mkl_res

    // tvm_res
}