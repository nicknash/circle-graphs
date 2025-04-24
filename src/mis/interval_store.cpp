#include "mis/interval_store.h"

namespace cg::mis
{
    IntervalStore::IntervalStore(int numIntervals) : _cmisToIntervals(numIntervals), _maxContainedMIS(0)
    {
        
    }
    void IntervalStore::addInterval(int containedMIS, cg::data_structures::Interval &interval)
    {
        if(containedMIS > _maxContainedMIS)
        {
            _maxContainedMIS = containedMIS;
        }

        if(containedMIS - _maxContainedMIS > 1)
        {
            throw std::runtime_error("WHAT");
        }
        _cmisToIntervals[containedMIS].push_front(interval);
    }

    std::optional<cg::data_structures::Interval> IntervalStore::tryGetRelevantInterval(int containedMIS, int left, int right)
    {
        auto bucket = _cmisToIntervals[containedMIS];
        int maxLeft = -1;
        std::optional<cg::data_structures::Interval> result;
        for(auto interval : bucket)
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

    int IntervalStore::getMaxContainedMIS()
    {
        return _maxContainedMIS;
    }

}