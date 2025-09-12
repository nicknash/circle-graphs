#pragma once

#include <span>

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

namespace cg::data_structures
{
    class DistinctIntervalModel;
    class Interval;
}

namespace cg::mif
{
template <class T>
    struct array4
    {
        std::size_t n;
        std::vector<T> data;
        array4() = default;
        explicit array4(std::size_t n) : n(n), data(n * n * n * n,T{}) {}
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
        std::size_t n;
        std::vector<T> data;
        explicit array3(std::size_t n) : n(n), data(n * n * n,T{}) {}
        T &operator()(std::size_t i, std::size_t j, std::size_t k)
        {
            return data[(i * n + j) * n + k];
        }
        const T &operator()(std::size_t i, std::size_t j, std::size_t k) const
        {
            return data[(i * n + j) * n + k];
        }
    };

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    class Gavril
    {
    public:
        static void computeMif(std::span<const cg::data_structures::Interval> intervals);
        static void computeRightForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<int>& rightForestSizes, array3<int>& dummyRightForestSizes);
        static void computeLeftForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<int>& leftForestSizes);
    };
}