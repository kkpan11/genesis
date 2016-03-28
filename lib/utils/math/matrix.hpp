#ifndef GENESIS_UTILS_MATH_MATRIX_H_
#define GENESIS_UTILS_MATH_MATRIX_H_

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include <stdexcept>
#include <vector>

namespace genesis {
namespace utils {

// =================================================================================================
//     Matrix
// =================================================================================================

template <typename T>
class Matrix
{
public:

    // -------------------------------------------------------------
    //     Typedefs
    // -------------------------------------------------------------

    typedef T                                       value_type;
    typedef typename std::vector<T>::iterator       iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;

    // -------------------------------------------------------------
    //     Constructors and Rule of Five
    // -------------------------------------------------------------

    Matrix (size_t rows, size_t cols) :
        rows_(rows),
        cols_(cols),
        data_(rows * cols)
    {}

    Matrix (size_t rows, size_t cols, T init) :
        rows_(rows),
        cols_(cols),
        data_(rows * cols, init)
    {}

    Matrix (size_t rows, size_t cols, std::initializer_list<T> const& init_list) :
        rows_(rows),
        cols_(cols),
        data_(rows * cols)
    {
        if (init_list.size() != size()) {
            throw std::length_error("__FUNCTION__: length_error");
        }

        size_t i = 0;
        for (T const& v : init_list) {
            data_[i] = v;
            ++i;
        }
    }

    ~Matrix() = default;

    Matrix(Matrix const&) = default;
    Matrix(Matrix&&)      = default;

    Matrix& operator= (Matrix const&) = default;
    Matrix& operator= (Matrix&&)      = default;

    void swap (Matrix& other)
    {
        using std::swap;
        swap(rows_, other.rows_);
        swap(cols_, other.cols_);
        swap(data_, other.data_);
    }

    // -------------------------------------------------------------
    //     Properties
    // -------------------------------------------------------------

    size_t rows() const
    {
        return rows_;
    }

    size_t cols() const
    {
        return cols_;
    }

    size_t size() const
    {
        return rows_ * cols_;
    }

    // -------------------------------------------------------------
    //     Element Access
    // -------------------------------------------------------------

    T& at (const size_t row, const size_t col)
    {
        if (row >= rows_ || col >= cols_) {
            throw std::out_of_range("__FUNCTION__: out_of_range");
        }
        return data_[row * cols_ + col];
    }

    const T at (const size_t row, const size_t col) const
    {
        if (row >= rows_ || col >= cols_) {
            throw std::out_of_range("__FUNCTION__: out_of_range");
        }
        return data_[row * cols_ + col];
    }

    T& operator () (const size_t row, const size_t col)
    {
        return data_[row * cols_ + col];
    }

    const T operator () (const size_t row, const size_t col) const
    {
        return data_[row * cols_ + col];
    }

    // -------------------------------------------------------------
    //     Iterators
    // -------------------------------------------------------------

    iterator begin()
    {
        return data_.begin();
    }

    iterator end()
    {
        return data_.end();
    }

    const_iterator begin() const
    {
        return data_.begin();
    }

    const_iterator end() const
    {
        return data_.end();
    }

    const_iterator cbegin() const
    {
        return data_.cbegin();
    }

    const_iterator cend() const
    {
        return data_.cend();
    }

    // -------------------------------------------------------------
    //     Operators
    // -------------------------------------------------------------

    bool operator == (const Matrix<T>& rhs) const
    {
        return rows_ == rhs.rows_
            && cols_ == rhs.cols_
            && data_ == rhs.data_;
    }

    bool operator != (const Matrix<T>& rhs) const
    {
        return !(*this == rhs);
    }

    // -------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------

private:

    size_t         rows_;
    size_t         cols_;
    std::vector<T> data_;
};

} // namespace utils
} // namespace genesis

// =================================================================================================
//     Namespace std Extension
// =================================================================================================

/*
namespace std {

template<typename T>
inline void swap (genesis::utils::Matrix<T>& lhs, genesis::utils::Matrix<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace std
*/

#endif // include guard
