#include <stdexcept>

#include "data_structures/interval.h"


#include "mis/interval_store.h"
#include <iostream>

namespace cg::mis
{
    IntervalStore::IntervalStore()
    {
        
    }
    void IntervalStore::addInterval(cg::data_structures::Interval &interval)
    {
        _intervals.push_front(interval);
    }

    std::optional<cg::data_structures::Interval> IntervalStore::tryGetRelevantInterval(int left, int right)
    {
        int maxLeft = -1;
        std::optional<cg::data_structures::Interval> result;
        for(auto interval : _intervals)
        {
            auto r = interval.Right + 1;
            if(r >= left && r < right  // right end point is in [left, right) 
            && interval.Left < left    // and its left endpoint isn't ALSO inside 
            && interval.Left > maxLeft) // and it's a new running max left end-point.
            {
                maxLeft = interval.Left;
                result = interval;
            }
        }
        return result;
    }
}