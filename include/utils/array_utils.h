#pragma once

#include <cstddef>
#include <vector>

namespace cg::utils
{
    template <class T>
    struct array4
    {
        std::size_t n;
        std::vector<T> data;
        array4() = default;
        explicit array4(std::size_t n, const T &empty) : n(n), data(n * n * n * n, empty) {}
        T &operator()(std::size_t i, std::size_t j, std::size_t k, std::size_t l)
        {
            return data[((i * n + j) * n + k) * n + l];
        }
        const T &operator()(std::size_t i, std::size_t j, std::size_t k, std::size_t l) const
        {
            return data[((i * n + j) * n + k) * n + l];
        }
    };

    template <class T>
    struct array3
    {
        int dim0 = 0;
        int dim1 = 0;
        int dim2 = 0;
        std::vector<T> data;

        array3() = default;

        explicit array3(int n, const T &empty)
            : dim0(n), dim1(n), dim2(n), data(static_cast<std::size_t>(n) * n * n, empty)
        {
        }

        array3(int d0, int d1, int d2, const T &empty)
            : dim0(d0), dim1(d1), dim2(d2), data(static_cast<std::size_t>(d0) * d1 * d2, empty)
        {
        }

        T &operator()(int i, int j, int k)
        {
            return data[static_cast<std::size_t>((i * dim1 + j) * dim2 + k)];
        }

        const T &operator()(int i, int j, int k) const
        {
            return data[static_cast<std::size_t>((i * dim1 + j) * dim2 + k)];
        }
    };

    template <class T>
    struct array2
    {
        int dim0 = 0;
        int dim1 = 0;
        std::vector<T> data;

        array2() = default;

        explicit array2(int n, const T &empty)
            : dim0(n), dim1(n), data(static_cast<std::size_t>(n) * n, empty)
        {
        }

        array2(int d0, int d1, const T &empty)
            : dim0(d0), dim1(d1), data(static_cast<std::size_t>(d0) * d1, empty)
        {
        }

        T &operator()(int i, int j)
        {
            return data[static_cast<std::size_t>(i * dim1 + j)];
        }

        const T &operator()(int i, int j) const
        {
            return data[static_cast<std::size_t>(i * dim1 + j)];
        }
    };
}

