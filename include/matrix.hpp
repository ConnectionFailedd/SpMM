#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <new>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

/* ------------------------------------------------------------------------------------------------------------------------ */

const auto ALIGN_CACHE_LINE = std::align_val_t(128);

/* ------------------------------------------------------------------------------------------------------------------------ */

template<class __Tp>
class DenseMatrix {
    template<class>
    friend class CSRMatrix;

public:
    class Line {
    private:
        std::size_t __size;
        __Tp * __basePtr;

    public:
        inline Line(std::size_t _size, __Tp * _basePtr) : __size(_size), __basePtr(_basePtr) {}

        inline const __Tp & operator[](std::size_t _index) const {
            if (_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");
            return __basePtr[_index];
        }
        inline __Tp & operator[](std::size_t _index) {
            if (_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");
            return __basePtr[_index];
        }
    };

    class ConstLine {
    private:
        std::size_t __size;
        __Tp * __basePtr;

    public:
        inline ConstLine(std::size_t _size, __Tp * _basePtr) : __size(_size), __basePtr(_basePtr) {}

        inline const __Tp & operator[](std::size_t _index) {
            if (_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");
            return __basePtr[_index];
        }
    };

private:
    std::size_t __lineNum, __columnNum;
    std::size_t __lineCapacity, __capacity;
    __Tp * __basePtr;

private:
    static std::size_t proper_line_capacity_of(std::size_t _columnNum) {
        auto lineCapacity = std::size_t(1);
        while (true) {
            if (lineCapacity >= _columnNum) return lineCapacity;
            if (lineCapacity == int(ALIGN_CACHE_LINE)) return (_columnNum / lineCapacity + 1) * lineCapacity;
            lineCapacity <<= 1;
        }
    }

public:
    inline DenseMatrix<__Tp>() : DenseMatrix<__Tp>(0, 0) {}
    inline DenseMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum) : __lineNum(_lineNum), __columnNum(_columnNum), __lineCapacity(proper_line_capacity_of(_columnNum)), __capacity(__lineNum * __lineCapacity), __basePtr(new(ALIGN_CACHE_LINE) __Tp[__capacity]()) {}
    DenseMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum, std::initializer_list<std::initializer_list<__Tp>> && _il) : __lineNum(_lineNum), __columnNum(_columnNum), __lineCapacity(proper_line_capacity_of(_columnNum)), __capacity(__lineNum * __lineCapacity) {
        __basePtr = new (ALIGN_CACHE_LINE) __Tp[__capacity]();

        auto lineIndex = std::size_t(0);
        auto lineIter = _il.begin();

        // if the shape of initializer doesn't fit the matrix, we cut off the waste part and complete the short lines with zeros
        while (lineIndex < __lineNum && lineIter != _il.end()) {
            auto columnIndex = std::size_t(0);
            auto columnIter = lineIter->begin();

            while (columnIndex < __columnNum && columnIter != lineIter->end()) {
                __basePtr[lineIndex * __lineCapacity + columnIndex] = *columnIter;

                columnIndex++;
                columnIter++;
            }

            lineIndex++;
            lineIter++;
        }
    }
    inline DenseMatrix<__Tp>(DenseMatrix<__Tp> && _src) noexcept : __lineNum(_src.__lineNum), __columnNum(_src.__columnNum), __capacity(_src.__capacity), __basePtr(_src.__basePtr) {
        _src.__basePtr = nullptr;
    }

    inline ~DenseMatrix<__Tp>() {
        if (__basePtr != nullptr) {
            delete[] __basePtr;
            __basePtr = nullptr;
        }
    }

    inline std::pair<std::size_t, std::size_t> size() const noexcept {
        return {__lineNum, __columnNum};
    }
    void resize(std::size_t _lineNum, std::size_t _columnNum) {
        __lineNum = _lineNum;
        __columnNum = _columnNum;
        __lineCapacity = proper_line_capacity_of(__columnNum);
        if (__lineNum * __lineCapacity > __capacity) {
            delete[] __basePtr;
            __capacity = __lineNum * __lineCapacity;
            __basePtr = new (ALIGN_CACHE_LINE) __Tp[__capacity]();
        }
    }

    inline ConstLine operator[](std::size_t _index) const {
        if (_index >= __lineNum) throw std::out_of_range("Subscription of matrix out of range.");
        return ConstLine(__columnNum, __basePtr + _index * __lineCapacity);
    }
    inline Line operator[](std::size_t _index) {
        if (_index >= __lineNum) throw std::out_of_range("Subscription of matrix out of range.");
        return Line(__columnNum, __basePtr + _index * __lineCapacity);
    }

    // friend std::ifstream & operator>>(std::ifstream & _ifs, DenseMatrix<__Tp> & _matrix) {
    //     // load from file
    //     alignas(std::max_align_t) char buffer[sizeof(std::max_align_t)];
    //     _ifs.read(buffer, sizeof(std::size_t));
    //     _matrix.__lineNum = *(int *)buffer;
    //     _ifs.read(buffer, sizeof(std::size_t));
    // }
    // friend std::ofstream & operator<<(std::ofstream & _ofs, const DenseMatrix<__Tp> & _matrix) {
    //     // save to file
    // }
};

/* ------------------------------------------------------------------------------------------------------------------------ */

template<class __Tp>
class CSRMatrix {
private:
    std::size_t __lineNum, __columnNum;
    std::vector<__Tp> __values;
    std::vector<std::size_t> __columnIndex;
    std::vector<std::size_t> __rowPtr;

public:
    CSRMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum)
        : __lineNum(_lineNum), __columnNum(_columnNum), __values(), __columnIndex(), __rowPtr(std::vector<std::size_t>(_lineNum + 1, 0)) {}
    CSRMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum, std::initializer_list<std::tuple<__Tp, std::size_t, std::size_t>> && _il)
        : __lineNum(_lineNum), __columnNum(_columnNum) {
        // this initialization should be safe
        auto entries = std::vector<std::tuple<__Tp, std::size_t, std::size_t>>(std::move(_il));
        auto cmp = [](const std::tuple<__Tp, std::size_t, std::size_t> & _lhs, const std::tuple<__Tp, std::size_t, std::size_t> & _rhs) {
            return std::get<1>(_lhs) < std::get<1>(_rhs);
        };
        std::sort(entries.begin(), entries.end(), cmp);
        __rowPtr.push_back(0);
        auto row = 0, count = 0;
        while (count < entries.size() && row < __lineNum) {
            auto entry = entries[count];
            if (std::get<1>(entry) != row) {
                __rowPtr.push_back(count);
                row++;
                continue;
            }
            __values.push_back(std::get<0>(entry));
            __columnIndex.push_back(std::get<2>(entry));
            count++;
        }
        __rowPtr.push_back(count);
    }

    // multiplication of CSR matrix and dense matrix
    // assume the scale of CSR matrix is m, the scale of dense matrix is n * p
    // the time complexity is O(m * p)
    DenseMatrix<__Tp> operator*(const DenseMatrix<__Tp> & _rhs) const {
        if (this->__columnNum != _rhs.__lineNum)
            throw "Invalid operands!";

        auto res = DenseMatrix<__Tp>(this->__lineNum, _rhs.__columnNum);
        for (auto lhsRow = 0; lhsRow < this->__lineNum; lhsRow++) {
            auto lhsRowBegin = this->__rowPtr[lhsRow];
            auto lhsRowEnd = this->__rowPtr[lhsRow + 1];
            for (auto lhsPtr = lhsRowBegin; lhsPtr < lhsRowEnd; lhsPtr++) {
                auto lhsColumn = this->__columnIndex[lhsPtr];
                auto lhsValue = this->__values[lhsPtr];
                for (auto rhsColumn = 0; rhsColumn < _rhs.__columnNum; rhsColumn++) {
                    auto rhsValue = _rhs.__matrix[lhsColumn][rhsColumn];
                    res.__matrix[lhsRow][rhsColumn] += lhsValue * rhsValue;
                }
            }
        }
        return res;
    }

    // friend std::ostream & operator<<(std::ostream & _ost, const CSRMatrix<__Tp> & _matrix) {
    //     for(auto row = 0; row < _matrix.__lineNum; row++) {
    //         auto rowBegin = _matrix.__rowPtr[row], rowEnd = _matrix.__rowPtr[row + 1];
    //         auto rowValues = std::vector<__Tp>(_matrix.__columnNum, __Tp());
    //         for(auto ptr = rowBegin; ptr < rowEnd; ptr++) {
    //             auto column = _matrix.__columnIndex[ptr];
    //             auto value = _matrix.__values[ptr];
    //             rowValues[column] = value;
    //         }
    //         for(auto column = 0; column < _matrix.__columnNum; column++) {
    //             _ost << rowValues[column] << "\t";
    //         }
    //         _ost << std::endl;
    //     }
    //     return _ost;
    // }

    // friend std::ifstream & operator>>(std::ifstream & _ist, CSRMatrix<__Tp> & _matrix) {
    //     // load from file
    // }
};

/* ------------------------------------------------------------------------------------------------------------------------ */

#define DEBUG
#ifdef DEBUG

template<class __Tp>
std::ostream & operator<<(std::ostream & _ost, const DenseMatrix<__Tp> & _matrix) {
    // print to command line
    auto size = _matrix.size();
    auto lineNum = size.first;
    auto columnNum = size.second;
    for (auto lineIndex = 0; lineIndex < lineNum; lineIndex++) {
        for (auto columnIndex = 0; columnIndex < columnNum; columnIndex++) {
            _ost << std::setw(7) << std::setfill(' ') << _matrix[lineIndex][columnIndex] << ' ';
        }
        _ost << std::endl;
    }
    return _ost;
}

#endif

/* ------------------------------------------------------------------------------------------------------------------------ */

#endif