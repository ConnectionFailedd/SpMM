#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <algorithm>
#include <cstddef>
#include <exception>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <new>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "debug.hpp"

/* ------------------------------------------------------------------------------------------------------------------------ */

const auto ALIGN_CACHE_LINE = std::align_val_t(128);

/* ------------------------------------------------------------------------------------------------------------------------ */

template<class __Tp>
class DenseMatrix {
public:
    class Line {
    private:
        std::size_t __size;
        __Tp * __basePtr;

    public:
        inline Line(std::size_t _size, __Tp * _basePtr) noexcept : __size(_size), __basePtr(_basePtr) {}

        inline const __Tp & operator[](std::size_t _index) const {
            if(_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");
            return __basePtr[_index];
        }
        inline __Tp & operator[](std::size_t _index) {
            if(_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");
            return __basePtr[_index];
        }
    };

    class ConstLine {
    private:
        std::size_t __size;
        __Tp * __basePtr;

    public:
        inline ConstLine(std::size_t _size, __Tp * _basePtr) noexcept : __size(_size), __basePtr(_basePtr) {}

        inline const __Tp & operator[](std::size_t _index) {
            if(_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");
            return __basePtr[_index];
        }
    };

private:
    std::size_t __lineNum, __columnNum;
    std::size_t __lineCapacity, __capacity; // unit: byte
    __Tp * __basePtr;

private:
    // unit: byte
    static std::size_t proper_line_capacity_of(std::size_t _columnNum) noexcept {
        auto lineCapacity = std::size_t(1);
        while(true) {
            if(lineCapacity >= _columnNum * sizeof(__Tp)) return lineCapacity;
            if(lineCapacity == std::size_t(ALIGN_CACHE_LINE)) return (_columnNum * sizeof(__Tp) / lineCapacity + 1) * lineCapacity;
            lineCapacity <<= 1;
        }
    }

public:
    inline DenseMatrix<__Tp>() : DenseMatrix<__Tp>(0, 0) {}
    DenseMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum, __Tp _initValue = __Tp()) : __lineNum(_lineNum), __columnNum(_columnNum), __lineCapacity(proper_line_capacity_of(_columnNum)), __capacity(__lineNum * __lineCapacity), __basePtr((__Tp *)operator new(__capacity, ALIGN_CACHE_LINE)) {
        // initialize
#pragma omp parallel for
        for(auto lineIndex = std::size_t(0); lineIndex < __lineNum; lineIndex++) {
            for(auto columnIndex = std::size_t(0); columnIndex < __columnNum; columnIndex++) {
                (*this)[lineIndex][columnIndex] = _initValue;
            }
        }
    }
    inline DenseMatrix<__Tp>(DenseMatrix<__Tp> && _src) noexcept : __lineNum(_src.__lineNum), __columnNum(_src.__columnNum), __lineCapacity(_src.__lineCapacity), __capacity(_src.__capacity), __basePtr(_src.__basePtr) {
        _src.__basePtr = nullptr;
    }

#if DEBUG == true
    DenseMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum, std::initializer_list<std::initializer_list<__Tp>> && _il) : DenseMatrix<__Tp>(_lineNum, _columnNum) {
        auto lineIndex = std::size_t(0);
        auto lineIter = _il.begin();

        // if the shape of initializer doesn't fit the matrix, we cut off the waste part and complete the short lines with __Tp()
        while(lineIndex < __lineNum && lineIter != _il.end()) {
            auto columnIndex = std::size_t(0);
            auto columnIter = lineIter->begin();

            while(columnIndex < __columnNum && columnIter != lineIter->end()) {
                (*this)[lineIndex][columnIndex] = *columnIter;

                columnIndex++;
                columnIter++;
            }

            while(columnIndex < __columnNum) {
                (*this)[lineIndex][columnIndex] = __Tp();

                columnIndex++;
            }

            lineIndex++;
            lineIter++;
        }

        while(lineIndex < __lineNum) {
            for(auto columnIndex = std::size_t(0); columnIndex < __columnNum; columnIndex++) {
                (*this)[lineIndex][columnIndex] = __Tp();
            }

            lineIndex++;
        }
    }
#endif

    inline ~DenseMatrix<__Tp>() {
        if(__basePtr != nullptr) {
            delete __basePtr;
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

        // if original capacity is smaller than needed, re-allocate memory
        if(__lineNum * __lineCapacity > __capacity) {
            delete __basePtr;
            __capacity = __lineNum * __lineCapacity;
            __basePtr = (__Tp *)operator new(__capacity, ALIGN_CACHE_LINE);
        }
    }

    inline ConstLine operator[](std::size_t _index) const {
        if(_index >= __lineNum) throw std::out_of_range("Subscription of matrix out of range.");
        return ConstLine(__columnNum, (__Tp *)((char *)__basePtr + _index * __lineCapacity));
    }
    inline Line operator[](std::size_t _index) {
        if(_index >= __lineNum) throw std::out_of_range("Subscription of matrix out of range.");
        return Line(__columnNum, (__Tp *)((char *)__basePtr + _index * __lineCapacity));
    }

    inline DenseMatrix<__Tp> & operator=(DenseMatrix<__Tp> && _rhs) noexcept {
        __lineNum = _rhs.__lineNum;
        __columnNum = _rhs.__columnNum;
        __lineCapacity = _rhs.__lineCapacity;
        __capacity = _rhs.__capacity;
        __basePtr = _rhs.__basePtr;

        _rhs.__basePtr = nullptr;

        return *this;
    }
};

/* ------------------------------------------------------------------------------------------------------------------------ */

template<class __Tp>
class CSRMatrix {
public:
    class ConstLine {
    private:
        std::size_t __size;
        typename std::vector<__Tp>::const_iterator __valueBegin;
        typename std::vector<std::size_t>::const_iterator __columnIndexBegin, __columnIndexEnd;

    public:
        inline static const __Tp ZERO = __Tp(0);

    public:
        ConstLine(std::size_t _size, const typename std::vector<__Tp>::const_iterator & _valueBegin, const typename std::vector<std::size_t>::const_iterator & _columnIndexBegin, const typename std::vector<std::size_t>::const_iterator & _columnIndexEnd) noexcept : __size(_size), __valueBegin(_valueBegin), __columnIndexBegin(_columnIndexBegin), __columnIndexEnd(_columnIndexEnd) {}

        inline const __Tp & operator[](std::size_t _index) const {
            if(_index >= __size) throw std::out_of_range("Subscription of one line in matrix out of range.");

            for(auto iter = __columnIndexBegin; iter != __columnIndexEnd; iter++) {
                if(*iter == _index) {
                    return *(__valueBegin + (iter - __columnIndexBegin));
                }
            }
            return ZERO;
        }
    };

private:
    std::size_t __lineNum, __columnNum;
    std::vector<__Tp> __values;
    std::vector<std::size_t> __columnIndexes;
    std::vector<std::size_t> __linePtrs;

public:
    inline CSRMatrix<__Tp>() : CSRMatrix<__Tp>(0, 0) {}
    inline CSRMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum) : __lineNum(_lineNum), __columnNum(_columnNum), __values(), __columnIndexes(), __linePtrs(std::vector<std::size_t>(2, 0)) {}

#if DEBUG == true
    CSRMatrix<__Tp>(std::size_t _lineNum, std::size_t _columnNum, std::initializer_list<std::tuple<__Tp, std::size_t, std::size_t>> && _il) : __lineNum(_lineNum), __columnNum(_columnNum), __values(), __columnIndexes(), __linePtrs(std::vector<std::size_t>(2, 0)) {
        // initializer list is COO matrix
        auto entries = std::vector<std::tuple<__Tp, std::size_t, std::size_t>>(std::move(_il));

        // sort by line index
        auto cmp = [](const std::tuple<__Tp, std::size_t, std::size_t> & _lhs, const std::tuple<__Tp, std::size_t, std::size_t> & _rhs) {
            return std::get<1>(_lhs) < std::get<1>(_rhs);
        };
        std::sort(entries.begin(), entries.end(), cmp);

        for(auto entry : entries) {
            try {
                push_back(entry);
            }
            catch(const std::exception & e) {
                return;
            }
        }
    }
#endif

    inline std::pair<std::size_t, std::size_t> size() const noexcept {
        return {__lineNum, __columnNum};
    }
    inline void resize(std::size_t _lineNum, std::size_t _columnNum) {
        // resize and clear all
        __lineNum = _lineNum;
        __columnNum = _columnNum;
        __values.clear();
        __columnIndexes.clear();
        __linePtrs = std::vector<std::size_t>(2, 0);
    }

    void push_back(const std::tuple<__Tp, std::size_t, std::size_t> & _cooEntry) {
        auto value = std::get<0>(_cooEntry);
        auto lineIndex = std::get<1>(_cooEntry);
        auto columnIndex = std::get<2>(_cooEntry);

        auto currentLineIndex = __linePtrs.size() - 2;

        if(lineIndex < currentLineIndex) throw std::invalid_argument("Push back value into previous lines.");
        if(lineIndex >= __lineNum) throw std::out_of_range("Push back value with position out of range.");

        // if new position of new value is in later lines
        while(lineIndex > currentLineIndex) {
            __linePtrs.push_back(__linePtrs.back());
            currentLineIndex++;
        }

        __values.push_back(value);
        __columnIndexes.push_back(columnIndex);
        __linePtrs.back()++;
    }

    ConstLine operator[](std::size_t _index) const {
        if(_index >= __lineNum) throw std::out_of_range("Subscription of matrix out of range.");

        auto currentLineIndex = __linePtrs.size() - 2;
        if(_index > currentLineIndex) return ConstLine(__columnNum, __values.end(), __columnIndexes.end(), __columnIndexes.end());

        auto valueBegin = __values.begin() + __linePtrs[_index];
        auto columnIndexBegin = __columnIndexes.begin() + __linePtrs[_index], columnIndexEnd = __columnIndexes.begin() + __linePtrs[_index + 1];
        return ConstLine(__columnNum, valueBegin, columnIndexBegin, columnIndexEnd);
    }

    // multiplication of CSR matrix and dense matrix
    // assume the scale of CSR matrix is m, the scale of dense matrix is n * p
    // the time complexity is O(m * p)
    DenseMatrix<__Tp> operator*(const DenseMatrix<__Tp> & _rhs) const {
        auto rhsSize = _rhs.size();
        auto rhsLineNum = rhsSize.first;
        auto rhsColumnNum = rhsSize.second;
        if(this->__columnNum != rhsLineNum) throw std::invalid_argument("Invalid matrix multiplication.");

        auto res = DenseMatrix<__Tp>(this->__lineNum, rhsColumnNum, 0);

#pragma omp parallel for schedule(dynamic, 1)
        for(auto lhsLineIndex = 0; lhsLineIndex < this->__lineNum; lhsLineIndex++) {
            if(lhsLineIndex >= this->__linePtrs.size() - 1) continue;

            // traverse lines of CSR matrix
            auto lhsLineBeginPtr = this->__linePtrs[lhsLineIndex];
            auto lhsLineEndPtr = this->__linePtrs[lhsLineIndex + 1];

            for(auto lhsLinePtr = lhsLineBeginPtr; lhsLinePtr < lhsLineEndPtr; lhsLinePtr++) {
                // CSR entries in lines
                auto lhsColumnIndex = this->__columnIndexes[lhsLinePtr];
                auto lhsValue = this->__values[lhsLinePtr];

                for(auto rhsColumnIndex = 0; rhsColumnIndex < rhsColumnNum; rhsColumnIndex++) {
                    // multiply every CSR entry with a line in dense matrix
                    auto rhsValue = _rhs[lhsColumnIndex][rhsColumnIndex];
                    res[lhsLineIndex][rhsColumnIndex] += lhsValue * rhsValue;
                }
            }
        }

        return res;
    }
};

/* ------------------------------------------------------------------------------------------------------------------------ */

template<class __Tp>
std::ifstream & operator>>(std::ifstream & _ifs, DenseMatrix<__Tp> & _denseMatrix) {
    // load from file
    auto bufferSize = sizeof(std::size_t) > sizeof(__Tp) ? sizeof(std::size_t) : sizeof(__Tp);
    alignas(std::max_align_t) char buffer[bufferSize];

    _ifs.read(buffer, sizeof(std::size_t));
    auto lineNum = *(std::size_t *)buffer;
    _ifs.read(buffer, sizeof(std::size_t));
    auto columnNum = *(std::size_t *)buffer;
    _denseMatrix.resize(lineNum, columnNum);

    for(auto lineIndex = std::size_t(0); lineIndex < lineNum; lineIndex++) {
        for(auto columnIndex = std::size_t(0); columnIndex < columnNum; columnIndex++) {
            _ifs.read(buffer, sizeof(__Tp));
            _denseMatrix[lineIndex][columnIndex] = *(__Tp *)buffer;
        }
    }
    return _ifs;
}

template<class __Tp>
std::ofstream & operator<<(std::ofstream & _ofs, const DenseMatrix<__Tp> & _denseMatrix) {
    auto bufferSize = sizeof(std::size_t) > sizeof(__Tp) ? sizeof(std::size_t) : sizeof(__Tp);
    alignas(std::max_align_t) char buffer[bufferSize];

    auto matrixSize = _denseMatrix.size();
    auto lineNum = matrixSize.first, columnNum = matrixSize.second;
    *(std::size_t *)buffer = lineNum;
    _ofs.write(buffer, sizeof(std::size_t));
    *(std::size_t *)buffer = columnNum;
    _ofs.write(buffer, sizeof(std::size_t));

    for(auto lineIndex = std::size_t(0); lineIndex < lineNum; lineIndex++) {
        for(auto columnIndex = std::size_t(0); columnIndex < columnNum; columnIndex++) {
            *(__Tp *)buffer = _denseMatrix[lineIndex][columnIndex];
            _ofs.write(buffer, sizeof(__Tp));
        }
    }
    return _ofs;
}

template<class __Tp>
std::ifstream & operator>>(std::ifstream & _ifs, CSRMatrix<__Tp> & _csrMatrix) {
    auto bufferSize = sizeof(std::size_t) > sizeof(__Tp) ? sizeof(std::size_t) : sizeof(__Tp);
    alignas(std::max_align_t) char buffer[bufferSize];

    _ifs.read(buffer, sizeof(std::size_t));
    auto lineNum = *(std::size_t *)buffer;
    _ifs.read(buffer, sizeof(std::size_t));
    auto columnNum = *(std::size_t *)buffer;
    _csrMatrix.resize(lineNum, columnNum);

    _ifs.read(buffer, sizeof(std::size_t));
    auto cooEntryNum = *(std::size_t *)buffer;

    for(auto index = 0; index < cooEntryNum; index++) {
        _ifs.read(buffer, sizeof(__Tp));
        auto value = *(__Tp *)buffer;
        _ifs.read(buffer, sizeof(std::size_t));
        auto lineIndex = *(std::size_t *)buffer;
        _ifs.read(buffer, sizeof(std::size_t));
        auto columnIndex = *(std::size_t *)buffer;
        _csrMatrix.push_back({value, lineIndex, columnIndex});
    }

    return _ifs;
}

/* ------------------------------------------------------------------------------------------------------------------------ */

#if DEBUG == true

// output to stdout for debugging

template<class __Tp>
std::ostream & operator<<(std::ostream & _ost, const DenseMatrix<__Tp> & _denseMatrix) {
    auto size = _denseMatrix.size();
    auto lineNum = size.first;
    auto columnNum = size.second;

    // loop to output
    for(auto lineIndex = 0; lineIndex < lineNum; lineIndex++) {
        for(auto columnIndex = 0; columnIndex < columnNum; columnIndex++) {
            _ost << std::setw(7) << std::setfill(' ') << _denseMatrix[lineIndex][columnIndex] << ' ';
        }
        _ost << std::endl;
    }
    return _ost;
}

template<class __Tp>
std::ostream & operator<<(std::ostream & _ost, const CSRMatrix<__Tp> & _csrMatrix) {
    auto size = _csrMatrix.size();
    auto lineNum = size.first;
    auto columnNum = size.second;

    // loop to output
    for(auto lineIndex = 0; lineIndex < lineNum; lineIndex++) {
        for(auto columnIndex = 0; columnIndex < columnNum; columnIndex++) {
            _ost << std::setw(7) << std::setfill(' ') << _csrMatrix[lineIndex][columnIndex] << ' ';
        }
        _ost << std::endl;
    }
    return _ost;
}

#endif

/* ------------------------------------------------------------------------------------------------------------------------ */

#endif