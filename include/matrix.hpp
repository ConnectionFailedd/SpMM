#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <tuple>
#include <vector>

template<class __Tp>
class DenseMatrix {
    template<class>
    friend class CSRMatrix;

private:
    std::size_t __rows, __columns;
    std::vector<std::vector<__Tp>> __matrix;

public:
    DenseMatrix<__Tp>(std::size_t _rows, std::size_t _columns) : __rows(_rows), __columns(_columns), __matrix(std::vector<std::vector<__Tp>>(_rows, std::vector<__Tp>(__columns, __Tp()))) {}
    DenseMatrix<__Tp>(std::size_t _rows, std::size_t _columns, std::initializer_list<std::initializer_list<__Tp>> && _il) : __rows(_rows), __columns(_columns) {
        __matrix = std::vector<std::vector<__Tp>>();
        auto rowValues = _il.begin();
        for(auto row = 0; row < __rows; row++) {
            __matrix.push_back(std::move(*rowValues));
            rowValues++;
        }
    }
    DenseMatrix<__Tp>(DenseMatrix<__Tp> && _denseMatrix) noexcept : __rows(_denseMatrix.__rows), __columns(_denseMatrix.__columns), __matrix(std::move(_denseMatrix.__matrix)) {}

    friend std::ostream & operator<<(std::ostream & _ost, const DenseMatrix<__Tp> & _matrix) {
        for(auto i = 0; i < _matrix.__rows; i++) {
            for(auto j = 0; j < _matrix.__columns; j++) {
                _ost << _matrix.__matrix[i][j] << "\t";
            }
            _ost << std::endl;
        }
        return _ost;
    }
};

template<class __Tp>
class CSRMatrix {
private:
    std::size_t __rows, __columns;
    std::vector<__Tp> __values;
    std::vector<std::size_t> __columnIndex;
    std::vector<std::size_t> __rowPtr;

public:
    CSRMatrix<__Tp>(std::size_t _rows, std::size_t _columns) : __rows(_rows), __columns(_columns), __values(), __columnIndex(), __rowPtr(std::vector<std::size_t>(_rows + 1, 0)) {}
    CSRMatrix<__Tp>(std::size_t _rows, std::size_t _columns, std::initializer_list<std::tuple<__Tp, std::size_t, std::size_t>> && _il) : __rows(_rows), __columns(_columns) {
        auto entries = std::vector<std::tuple<__Tp, std::size_t, std::size_t>>(std::move(_il));
        auto cmp = [](const std::tuple<__Tp, std::size_t, std::size_t> & _lhs, const std::tuple<__Tp, std::size_t, std::size_t> & _rhs) { return std::get<1>(_lhs) < std::get<1>(_rhs); };
        std::sort(entries.begin(), entries.end(), cmp);
        __rowPtr.push_back(0);
        for(auto i = 0, row = 0; i < entries.size(); i++) {
            auto entry = entries[i];
            if(std::get<1>(entry) != row) {
                __rowPtr.push_back(i);
                row++;
            }
            __values.push_back(std::get<0>(entry));
            __columnIndex.push_back(std::get<2>(entry));
        }
        __rowPtr.push_back(entries.size());
    }

    // multiplication of CSR matrix and dense matrix
    DenseMatrix<__Tp> operator*(const DenseMatrix<__Tp> & _rhs) const {
        if(this->__columns != _rhs.__rows) throw "Invalid operands!";

        auto res = DenseMatrix<__Tp>(this->__rows, _rhs.__columns);
        for(auto lhsRow = 0; lhsRow < this->__rows; lhsRow++) {
            auto lhsRowBegin = this->__rowPtr[lhsRow];
            auto lhsRowEnd = this->__rowPtr[lhsRow + 1];
            for(auto lhsPtr = lhsRowBegin; lhsPtr < lhsRowEnd; lhsPtr++) {
                auto lhsColumn = this->__columnIndex[lhsPtr];
                auto lhsValue = this->__values[lhsPtr];
                for(auto rhsColumn = 0; rhsColumn < _rhs.__columns; rhsColumn++) {
                    auto rhsValue = _rhs.__matrix[lhsColumn][rhsColumn];
                    res.__matrix[lhsRow][rhsColumn] += lhsValue * rhsValue;
                }
            }
        }
        return res;
    }

    friend std::ostream & operator<<(std::ostream & _ost, const CSRMatrix<__Tp> & _matrix) {
        for(auto row = 0; row < _matrix.__rows; row++) {
            auto rowBegin = _matrix.__rowPtr[row], rowEnd = _matrix.__rowPtr[row + 1];
            auto rowValues = std::vector<__Tp>(_matrix.__columns, __Tp());
            for(auto ptr = rowBegin; ptr < rowEnd; ptr++) {
                auto column = _matrix.__columnIndex[ptr];
                auto value = _matrix.__values[ptr];
                rowValues[column] = value;
            }
            for(auto column = 0; column < _matrix.__columns; column++) {
                _ost << rowValues[column] << "\t";
            }
            _ost << std::endl;
        }
        return _ost;
    }
};

#endif