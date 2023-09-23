#include "matrix.hpp"

#include <fstream>
#include <iostream>

int main() {
    // auto csr = CSRMatrix<int>(3, 2, {{1, 0, 0}, {2, 2, 1}, {3, 2, 0}, {4, 3, 0}});
    // std::cout << "CSR: matrix" << std::endl;
    // std::cout << csr << std::endl;

    auto dense = DenseMatrix<int>(2, 3, {{1, 2, 3, 4}, {5, 6}});
    std::cout << "Dense matrix:" << std::endl;
    std::cout << dense << std::endl;

    // auto res = csr * dense;

    // std::cout << "Result:" << std::endl;
    // std::cout << res << std::endl;
    // std::ofstream a;
}