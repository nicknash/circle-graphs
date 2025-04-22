#pragma once

#include <stack>
#include <vector>
#include <optional>

#include <map>
#include <limits>

namespace cg::mis
{
    class IndependentSet;
    class ImplicitIndependentSet;
}

namespace cg::data_structures
{
    class DistinctIntervalRep;
    class Interval;
}

namespace cg::mis::distinct
{
    class MonotoneSeq
    {
    private:
        std::map<int, int> _rangeLeftToValue;
        int _size;
        void validateIndex(int idx)
        {
            if (idx < 0 || idx >= _size)
            {
                throw std::out_of_range(std::format("Index {} is out of range, should be between 0 and {} inclusive", idx, _size - 1));
            }
        }
        std::map<int, int>::iterator getRangeIter(int idx) // Largest key <= idx
        {
            auto it = _rangeLeftToValue.upper_bound(idx); // This gives us the smallest key > idx, or end()
            --it;
            return it;
        }

    public:
        struct Range
        {
            int left;
            int right;
        };

        MonotoneSeq(int size)
        {
            _size = size;
            _rangeLeftToValue.emplace(-1, std::numeric_limits<int>::max());
            _rangeLeftToValue.emplace(0, 0);
            _rangeLeftToValue.emplace(_size, -1);
        }

        [[nodiscard]] int get(int idx)
        {
            validateIndex(idx);
            auto it = getRangeIter(idx);
            auto valueHere = it->second;
            return valueHere;
        }

        Range increment(int idx)
        {
            validateIndex(idx);

            auto it = getRangeIter(idx);
            auto rangeStartForIdx = it->first;
            auto oldValue = it->second;


            --it;
            auto prevRangeStartIdx = it->first;
            auto prevRangeValue = it->second;

            ++it;
            ++it;
            auto nextRangeStartIdx = it->first;
            --it;

            int endChange;
            if (idx + 1 < nextRangeStartIdx)
            {
                _rangeLeftToValue.emplace(idx + 1, oldValue);
                endChange = idx + 1;
            }
            else
            {
                endChange = nextRangeStartIdx;
            }
            if(prevRangeValue == oldValue + 1)
            {
                _rangeLeftToValue.erase(rangeStartForIdx);
            }
            else
            {
                ++it->second;
            }
            return Range{rangeStartForIdx, endChange};
        }

        void copyTo(std::vector<int>& target)
        {
            auto it = _rangeLeftToValue.begin();
            ++it; // Skip sentinel.
            auto prev = it;
            ++it;
            int idx = 0;
            while(it != _rangeLeftToValue.end())
            {
                for(auto i = prev->first; i < it->first; ++i)
                {
                    target[idx] = prev->second; 
                    ++idx;
                }
                prev = it;
                ++it;
            }
        }
    };

    class ImplicitOutputSensitive
    {
    private:
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
        static bool tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, cg::mis::ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &interval, MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS);
    };
}
