#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

int main(int argc, char ** argv) {
    auto density = 0.001;
    auto m = std::size_t(1024), n = std::size_t(1024), p = std::size_t(1024);

    // parse arguments
    for(auto index = std::size_t(1); index < argc; index++) {
        if(argv[index] == std::string("-d") || argv[index] == std::string("--density")) {
            density = std::stod(argv[index + 1]);
            index++;
        }
        else if(argv[index] == std::string("-m")) {
            m = std::stoul(argv[index + 1]);
        }
        else if(argv[index] == std::string("-n")) {
            n = std::stoul(argv[index + 1]);
        }
        else if(argv[index] == std::string("-p")) {
            p = std::stoul(argv[index + 1]);
        }
        else {
            std::cout << "Testcase generator for SpMM." << std::endl;
            std::cout << "Usage: TESTCASE_GEN [-d <density>] [-m <dimension-1>] [-n <dimension-2>] [-p <dimension-3>]" << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "    -d, --density       the density of sparse matrix." << std::endl;
            std::cout << "    -m, -n, -p          three dimensions of two matrix." << std::endl;
            return 0;
        }
    }
    std::cout << "Executing with arguments:   -d " << density << " -m " << m << " -n " << n << " -p " << p << std::endl;

    // random real numbers in (0, 1) generator
    std::default_random_engine randomEngine;
    std::uniform_real_distribution<double> uniformDistribution(0, 1);

    // buffer for writing to file
    auto bufferSize = sizeof(std::size_t) > sizeof(double) ? sizeof(std::size_t) : sizeof(double);
    char buffer[bufferSize];

    // create directory
    system("mkdir -p testcases/sparse");
    system("mkdir -p testcases/dense");

    // generate sparse matrix file (COO format)
    auto sparseFile = std::ofstream("testcases/sparse/sparse_" + std::to_string(m) + 'x' + std::to_string(n), std::ios::out | std::ios::binary);
    auto cooEntries = std::vector<std::tuple<double, std::size_t, std::size_t>>();
    for(auto lineIndex = 0; lineIndex < m; lineIndex++) {
        for(auto columnIndex = 0; columnIndex < n; columnIndex++) {
            if(uniformDistribution(randomEngine) < density) {
                cooEntries.push_back({uniformDistribution(randomEngine), lineIndex, columnIndex});
            }
        }
    }
    *(std::size_t *)buffer = m;
    sparseFile.write(buffer, sizeof(std::size_t));
    *(std::size_t *)buffer = n;
    sparseFile.write(buffer, sizeof(std::size_t));
    *(std::size_t *)buffer = cooEntries.size();
    sparseFile.write(buffer, sizeof(std::size_t));
    for(auto cooEntry : cooEntries) {
        *(double *)buffer = std::get<0>(cooEntry);
        sparseFile.write(buffer, sizeof(double));
        *(std::size_t *)buffer = std::get<1>(cooEntry);
        sparseFile.write(buffer, sizeof(std::size_t));
        *(std::size_t *)buffer = std::get<2>(cooEntry);
        sparseFile.write(buffer, sizeof(std::size_t));
    }

    // generate dense matrix file
    auto denseFile = std::ofstream("testcases/dense/dense_" + std::to_string(n) + 'x' + std::to_string(p), std::ios::out | std::ios::binary);
    *(std::size_t *)buffer = n;
    denseFile.write(buffer, sizeof(std::size_t));
    *(std::size_t *)buffer = p;
    denseFile.write(buffer, sizeof(std::size_t));
    for(auto lineIndex = 0; lineIndex < n; lineIndex++) {
        for(auto columnIndex = 0; columnIndex < p; columnIndex++) {
            *(double *)buffer = uniformDistribution(randomEngine);
            denseFile.write(buffer, sizeof(double));
        }
    }
}