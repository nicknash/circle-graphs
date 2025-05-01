#include "mis/monotone_seq.h"

namespace cg::mis
{
    void UnitMonotoneSeq::validateIndex(int idx)
    {
        if (idx < 0 || idx >= _size)
        {
            throw std::out_of_range(std::format("Index {} is out of range, should be between 0 and {} inclusive", idx, _size - 1));
        }
    }
    std::map<int, int>::iterator UnitMonotoneSeq::getRangeIter(int idx) // Largest key <= idx
    {
        auto it = _rangeLeftToValue.upper_bound(idx); // This gives us the smallest key > idx, or end()
        --it;
        return it;
    }

    UnitMonotoneSeq::UnitMonotoneSeq(int size)
    {
        _size = size;
        _rangeLeftToValue.emplace(-1, std::numeric_limits<int>::max());
        _rangeLeftToValue.emplace(0, 0);
        _rangeLeftToValue.emplace(_size, -1);
    }

    [[nodiscard]] int UnitMonotoneSeq::get(int idx)
    {
        validateIndex(idx);
        auto it = getRangeIter(idx);
        auto valueHere = it->second;
        return valueHere;
    }

    UnitMonotoneSeq::Range UnitMonotoneSeq::increment(int idx)
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

        int newEndRange;
        if (idx + 1 < nextRangeStartIdx)
        {
            _rangeLeftToValue.emplace(idx + 1, oldValue);
            newEndRange = idx + 1;
        }
        else
        {
            newEndRange = nextRangeStartIdx;
        }

        auto newBeginRange = rangeStartForIdx;
        if (prevRangeValue == oldValue + 1)
        {
            newBeginRange = prevRangeStartIdx;
            _rangeLeftToValue.erase(rangeStartForIdx);
        }
        else
        {
            ++it->second;
        }
        return Range{newBeginRange, rangeStartForIdx, newEndRange};
    }

    void UnitMonotoneSeq::copyTo(std::vector<int> &target)
    {
        auto it = _rangeLeftToValue.begin();
        ++it; // Skip sentinel.
        auto prev = it;
        ++it;
        int idx = 0;
        while (it != _rangeLeftToValue.end())
        {
            for (auto i = prev->first; i < it->first; ++i)
            {
                target[idx] = prev->second;
                ++idx;
            }
            prev = it;
            ++it;
        }
    }
}
