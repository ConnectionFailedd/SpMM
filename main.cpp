#include "matrix.hpp"

#include <fstream>
#include <iostream>

int main() {
    auto csr = CSRMatrix<double>(3, 3, {{1, 0, 0}, {2, 1, 1}, {3, 1, 0}, {4, 3, 0}});
    // std::ifstream("testcase/csr", std::ios::binary | std::ios::in) >> csr;
    std::cout << "CSR: matrix" << std::endl;
    std::cout << csr << std::endl;

    auto dense = DenseMatrix<double>(3, 3, {{1, 2, 3, 4}, {5, 6}});
    // std::ifstream("testcase/dense", std::ios::binary | std::ios::in) >> dense;
    // std::ofstream("testcase/dense1", std::ios::binary | std::ios::out) << dense;
    std::cout << "Dense matrix:" << std::endl;
    std::cout << dense << std::endl;

    auto res = csr * dense;

    std::cout << "Result:" << std::endl;
    std::cout << res << std::endl;
}