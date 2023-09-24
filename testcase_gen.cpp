#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

int main(int argc, char ** argv) {
    auto testcaseNum = std::size_t(128);
    auto density = 0.001;
    auto m = std::size_t(1024), n = std::size_t(1024), p = std::size_t(1024);

    if(argc == 1) {
        std::cout << "Testcase generator for SpMM." << std::endl;
        std::cout << "Executing with default arguments:   -N " << testcaseNum << " -D " << density << " -m " << m << " -n " << n << " -p " << p << std::endl;
        std::cout << "Usage: testcase_gen [-N <testcase-number>] [-D <density>] [-m <dimension-1>] [-n <dimension-2>] [-p <dimension-3>]" << std::endl;
        std::cout << "Arguments:" << std::endl;
        std::cout << "    -N, --number        the number of testcases to be generated." << std::endl;
        std::cout << "    -D, --density       the density of sparse matrix." << std::endl;
        std::cout << "    -m, -n, -p          three dimensions of two matrix." << std::endl;
    }

    // parse arguments
    for(auto index = std::size_t(1); index < argc; index++) {
        if(argv[index] == std::string("-N") || argv[index] == std::string("--number")) {
            testcaseNum = std::stoul(argv[index + 1]);
            index++;
        }
        else if(argv[index] == std::string("-D") || argv[index] == std::string("--density")) {
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
            std::cout << "Usage: testcase_gen [-N <testcase-number>] [-D <density>] [-m <dimension-1>] [-n <dimension-2>] [-p <dimension-3>]" << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "    -N, --number        the number of testcases to be generated." << std::endl;
            std::cout << "    -D, --density       the density of sparse matrix." << std::endl;
            std::cout << "    -m, -n, -p          three dimensions of two matrix." << std::endl;
        }
    }

    // random real numbers in (0, 1) generator
    std::default_random_engine randomEngine;
    std::uniform_real_distribution<double> uniformDistribution(0, 1);

    // buffer for writing to file
    auto bufferSize = sizeof(std::size_t) > sizeof(double) ? sizeof(std::size_t) : sizeof(double);
    char buffer[bufferSize];

    // create directory
    system("mkdir -p testcases");
    for(auto index = std::size_t(0); index < testcaseNum; index++) {
        // generate sparse matrix file (COO format)
        auto sparseFile = std::ofstream("testcases/sparse" + std::to_string(index), std::ios::out | std::ios::binary);
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
        sparseFile.close();

        // generate dense matrix file
        auto denseFile = std::ofstream("testcases/dense" + std::to_string(index), std::ios::out | std::ios::binary);
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
}