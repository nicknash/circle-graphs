#pragma once

#include <map>
#include <vector>
#include <limits>

namespace cg::mis
{
    class MonotoneSeq
    {
    private:
        std::vector<int> _values; // TODO use a logarithmic data structure.
        int _size;
    public:
        struct Range
        {
            int changeStartInclusive; 
            int changeEndExclusive;
        };
        MonotoneSeq(int size);
        [[nodiscard]] int get(int idx);
        Range set(int idx, int value);
        void copyTo(std::vector<int>& target);
    };
}
