#include <stdexcept>
#include <format>

#include "mis/monotone_seq.h"

namespace cg::mis
{
   
    MonotoneSeq::MonotoneSeq(int size) : _values(size + 1), _size(size)
    {
     
    }

    [[nodiscard]] int MonotoneSeq::get(int idx)
    {
        return _values[idx];
    }

    MonotoneSeq::Range MonotoneSeq::set(int idx, int value)
    {
        if (idx < 0 || idx >= _size)
        {
            throw std::out_of_range(std::format("Index {} is out of range, should be between 0 and {} inclusive", idx, _size - 1));
        }
        if(_values[idx] >= value)
        {
            throw std::out_of_range(std::format("Cannot set value {} at index {} to value, {} that is not strictly larger", _values[idx], idx, value));
        }
        auto i = idx;
        while(i >= 0 && value > _values[i])
        {
            _values[i] = value;
            --i;
        }

        return Range{i + 1, idx + 1};
    }

    void MonotoneSeq::copyTo(std::vector<int> &target)
    {
        auto last = _values.end();
        --last;
        target.assign(_values.begin(), last);
    }
}
