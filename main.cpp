#include "matrix.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
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
    std::cout << "Executing with default arguments:   -M " << measureTime << " -t " << threadNum << " -m " << m << " -n " << n << " -p " << p << std::endl;

    omp_set_num_threads(threadNum);

    auto timeReport = std::ofstream();
    auto totalTime = std::chrono::nanoseconds::rep(0);
    auto repeatTimes = std::size_t(0);

    // my_res
    system("mkdir -p testcases/my_res");

    auto csr = CSRMatrix<double>();
    auto dense = DenseMatrix<double>();
    auto res = DenseMatrix<double>();
    std::ifstream("testcases/sparse/sparse_" + std::to_string(m) + 'x' + std::to_string(n), std::ios::in | std::ios::binary) >> csr;
    std::ifstream("testcases/dense/dense_" + std::to_string(n) + 'x' + std::to_string(p), std::ios::in | std::ios::binary) >> dense;

    timeReport.open("testcases/my_res/time_" + std::to_string(m) + 'x' + std::to_string(n) + 'x' + std::to_string(p) + "_t" + std::to_string(threadNum), std::ios::out | std::ios::trunc);
    totalTime = 0;
    repeatTimes = 0;

    timeReport << "Time(ns):" << std::endl;
    while(totalTime <= measureTime) {
        // start timing
        auto start = std::chrono::high_resolution_clock::now();

        // multiplication
        res = csr * dense;

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
    std::ofstream("testcases/my_res/res_" + std::to_string(m) + 'x' + std::to_string(n) + 'x' + std::to_string(p), std::ios::out | std::ios::binary) << res;
    timeReport.close();

    // mkl_res

    // tvm_res
}