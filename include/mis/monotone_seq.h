#pragma once

#include <map>
#include <vector>
#include <limits>
#include <stdexcept>
#include <format>

namespace cg::mis
{
    class MonotoneSeq
    {
    private:
        std::map<int, int> _rangeLeftToValue;
        int _size;
        void validateIndex(int idx);
        std::map<int, int>::iterator getRangeIter(int idx); // Largest key <= idx
    public:
        struct Range
        {
            int left;
            int changePoint;
            int right;
        };
        MonotoneSeq(int size);
        [[nodiscard]] int get(int idx);
        Range increment(int idx);
        void copyTo(std::vector<int>& target);
    };
}
