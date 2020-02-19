#pragma once

#include <QtGlobal>

#include <array>

// Square matrix N x N
template<typename T, size_t N>
class Matrix
{
public:
    using value_type = T;

    Matrix();
    Matrix(const T* ip_values);
    Matrix(const Matrix<T,N>& i_matrix);

    virtual ~Matrix() = default;

    T& Item(size_t i_index);
    const T& Item(size_t i_index) const;

    T& Item(size_t i_row, size_t i_col);
    const T& Item(size_t i_row, size_t i_col) const;

    T& operator()(size_t i_row, size_t i_col);
    const T& operator()(size_t i_row, size_t i_col) const;

    bool operator==(const Matrix<T, N>& i_matrix) const;

    Matrix<T, N>& operator=(const Matrix<T, N>& i_matrix);
    
    Matrix<T, N>& operator+=(const Matrix<T, N>& i_matrix);
    Matrix<T, N>& operator-=(const Matrix<T, N>& i_matrix);

    template<typename TFactor>
    Matrix<T, N>& operator*=(const TFactor& i_factor);
    template<typename TDivisor>
    Matrix<T, N>& operator/=(const TDivisor& i_divisor);

    //Matrix<T, N> operator+(const Matrix<T, N>& i_matrix) const;
    //Matrix<T, N> operator-(const Matrix<T, N>& i_matrix) const;

    void MakeIdentity();

private:
    T* _GetDataPtr();
    const T* const _GetDataPtr() const;

private:
    std::array<T, N * N> m_data;
};

template<typename T, size_t N>
inline Matrix<T, N>::Matrix()
{
    MakeIdentity();
}

template<typename T, size_t N>
inline Matrix<T, N>::Matrix(const T* ip_values)
{
    memcpy(_GetDataPtr(), ip_values, N * N * sizeof(T));
}

template<typename T, size_t N>
inline Matrix<T, N>::Matrix(const Matrix<T,N>& i_matrix)
    : Matrix<T,N>(i_matrix._GetDataPtr())
{
}

template<typename T, size_t N>
inline T& Matrix<T, N>::Item(size_t i_index)
{
    Q_ASSERT(i_index >= 0 && i_index < N * N);
    return m_data[i_index];
}

template<typename T, size_t N>
inline const T& Matrix<T, N>::Item(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < N * N);
    return m_data[i_index];
}

template<typename T, size_t N>
inline T& Matrix<T, N>::Item(size_t i_row, size_t i_col)
{
    return Item(N * i_row + i_col);
}

template<typename T, size_t N>
inline const T& Matrix<T, N>::Item(size_t i_row, size_t i_col) const
{
    return Item(N * i_row + i_col);
}

template<typename T, size_t N>
inline T& Matrix<T, N>::operator()(size_t i_row, size_t i_col)
{
    return Item(i_row, i_col);
}

template<typename T, size_t N>
inline const T& Matrix<T, N>::operator()(size_t i_row, size_t i_col) const
{
    return Item(i_row, i_col);
}

template<typename T, size_t N>
inline bool Matrix<T, N>::operator==(const Matrix<T, N>& i_matrix) const
{
    for (size_t i = 0; i < N * N; ++i)
        if (Item(i) != i_matrix.Item(i))
            return false;
    return true;
}

template<typename T, size_t N>
inline Matrix<T, N>& Matrix<T, N>::operator=(const Matrix<T, N>& i_matrix)
{
    if (this == &i_matrix)
        return *this;

    memcpy(_GetDataPtr(), i_matrix._GetDataPtr(), N * N * sizeof(T));
    return *this;
}

template<typename T, size_t N>
inline Matrix<T, N>& Matrix<T, N>::operator+=(const Matrix<T, N>& i_matrix)
{
    *this = *this + i_matrix;
    return *this;
}

template<typename T, size_t N>
inline Matrix<T, N>& Matrix<T, N>::operator-=(const Matrix<T, N>& i_matrix)
{
    *this = *this - i_matrix;
    return *this;
}

/*
template<typename T, size_t N>
inline Matrix<T, N> Matrix<T, N>::operator+(const Matrix<T, N>& i_matrix) const
{
    Matrix<T, N> result(_GetDataPtr());
    for (size_t i = 0; i < N * N; ++i)
        result.Item(i) += i_matrix.Item(i);
    return result;
}

template<typename T, size_t N>
inline Matrix<T, N> Matrix<T, N>::operator-(const Matrix<T, N>& i_matrix) const
{
    Matrix<T, N> result(_GetDataPtr());
    for (size_t i = 0; i < N * N; ++i)
        result.Item(i) -= i_matrix.Item(i);
    return result;
}
*/

template<typename T, size_t N>
inline void Matrix<T, N>::MakeIdentity()
{
    std::fill(std::begin(m_data), std::end(m_data), T{});
    for (size_t i = 0; i < N; ++i)
        Item(i * N + i) = static_cast<T>(1);
}

template<typename T, size_t N>
inline T* Matrix<T, N>::_GetDataPtr() 
{
    return m_data.data();
}
template<typename T, size_t N>
inline const T* const Matrix<T, N>::_GetDataPtr() const 
{
    return m_data.data();
}

template<typename T, size_t N>
template<typename TFactor>
inline Matrix<T, N>& Matrix<T, N>::operator*=(const TFactor& i_factor)
{
    *this = (*this) * i_factor;
    return *this;
}

template<typename T, size_t N>
template<typename TDivisor>
inline Matrix<T, N>& Matrix<T, N>::operator/=(const TDivisor& i_divisor)
{
    *this = (*this) / i_divisor;
    return *this;
}

/*
template<typename T, size_t N>
template<typename TFactor>
inline Matrix<T, N> Matrix<T, N>::operator*(const TFactor& i_factor) const
{
    for (size_t i = 0; i < N * N; ++i)
        Item(i) *= i_factor;
}

template<typename T, size_t N>
template<typename TDivisor>
inline Matrix<T, N> Matrix<T, N>::operator/(const TDivisor& i_divisor) const
{
    for (size_t i = 0; i < N * N; ++i)
        Item(i) /= i_divisor;
}
*/

// free functions

template<typename T, size_t N>
inline Matrix<T, N> operator+(const Matrix<T, N>& i_lhs, const Matrix<T, N>& i_rhs)
{
    Matrix<T, N> result(i_lhs);
    result += i_rhs;
    return result;
}

template<typename T, size_t N>
inline Matrix<T, N> operator-(const Matrix<T, N>& i_lhs, const Matrix<T, N>& i_rhs)
{
    Matrix<T, N> result(i_lhs);
    result -= i_rhs;
    return result;
}

template<typename T, size_t N>
inline Matrix<T, N> operator*(const Matrix<T, N>& i_lhs, const T& i_rhs)
{
    Matrix<T, N> result(i_lhs);
    result *= i_rhs;
    return result;
}

template<typename T, size_t N>
inline Matrix<T, N> operator*(const T& i_lhs, const Matrix<T, N>& i_rhs)
{
    return i_rhs * i_lhs;
}

template<typename T, size_t N>
inline Matrix<T, N> operator/(const Matrix<T, N>& i_lhs, const T& i_rhs)
{
    Matrix<T, N> result(i_lhs);
    result /= i_rhs;
    return result;
}

template<typename T, size_t N>
inline Matrix<T, N> operator/(const T& i_lhs, const Matrix<T, N>& i_rhs)
{
    return i_rhs / i_lhs;
}

template<typename T, size_t N>
inline Matrix<T, N> operator*(const Matrix<T, N>& i_lhs, const Matrix<T, N>& i_rhs)
{
    Matrix<T, N> result;
    for (size_t i = 0; i < N; ++i)
        result.Item(i, i) = T{};

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            for (size_t k = 0; k < N; ++k)
            {
                result.Item(i, j) += i_lhs.Item(i, k) * i_rhs.Item(k, j);
            }
        }
    }

    return result;
}
