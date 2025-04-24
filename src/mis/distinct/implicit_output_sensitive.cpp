#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/monotone_seq.h"

#include "mis/distinct/implicit_output_sensitive.h"

#include <iostream>

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

namespace cg::mis::distinct
{
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::MonotoneSeq &MIS, cg::mis::IntervalStore &intervalStore, int maxAllowedMIS)
    {
        pendingUpdates.push(newInterval);

        while (!pendingUpdates.empty())
        {
            auto currentInterval = pendingUpdates.top();
            pendingUpdates.pop();

            cg::mis::MonotoneSeq::Range r = MIS.increment(currentInterval.Left);
            
            independentSet.setRange(r.left, r.right - 1, currentInterval);
 
            auto maxContainedMIS = intervalStore.getMaxContainedMIS();
            auto representativeMIS = MIS.get(r.changePoint);
            for(auto cmis = 0; cmis <= maxContainedMIS; ++cmis)
            {
                auto maybeInterval = intervalStore.tryGetRelevantInterval(cmis, r.changePoint, r.right); 
                if (maybeInterval)
                {
                    auto interval = maybeInterval.value();
                    auto candidate = interval.Weight + cmis + representativeMIS; 
                    if (candidate > maxAllowedMIS)
                    {
                        return false;
                    }
                    if (candidate > MIS.get(interval.Left))
                    {
                        pendingUpdates.push(interval);
                    }
                }
            }

//if(d < maxContainedMIS) can do this interval iteration instead taking O(d log n) time. 
/*
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.right);
            while(maybeInterval) 
            {
                auto interval = maybeInterval.value();

                if(interval.Right < r.left)
                {
                    break;
                }

                auto candidate = interval.Weight + CMIS[interval.Index] + MIS.get(interval.Right + 1);
                if (candidate > maxAllowedMIS)
                {
                    return false;
                }

                auto thisRegionValue = MIS.get(interval.Left); 
                if (candidate > thisRegionValue)
                {
                    pendingUpdates.push(interval.Left);
                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
            }

*/

        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> ImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS)
    {
        std::stack<cg::data_structures::Interval> pendingUpdates;
        cg::mis::MonotoneSeq MIS(intervals.end);
        cg::mis::IntervalStore intervalStore(intervals.size);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                intervalStore.addInterval(MIS.get(interval.Left + 1), interval);
                //CMIS[interval.Index] = MIS.get(interval.Left + 1);
                independentSet.assembleContainedIndependentSet(interval);
                if (!tryUpdate(intervals, pendingUpdates, independentSet, interval, MIS, intervalStore, maxAllowedMIS))
                {
                    return std::nullopt;
                }
            }
        }

        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));
        return intervalsInMis;
    }
}
