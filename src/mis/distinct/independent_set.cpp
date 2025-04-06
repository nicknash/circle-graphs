#include "data_structures/interval.h"
#include "data_structures/distinct_interval_rep.h"

#include "mis/distinct/independent_set.h"

namespace cg::mis::distinct
{
    IndependentSet::IndependentSet(const cg::data_structures::DistinctIntervalRep& intervals) : 
        _intervals(intervals),
        _intervalIndexToNextRightEndpoint(intervals.end + 1), 
        _intervalIndexToDirectlyContained(intervals.end)
    {

    }

    void IndependentSet::setSameNextRightEndpoint(int where)
    {
        _intervalIndexToNextRightEndpoint[where] = _intervalIndexToNextRightEndpoint[where + 1];
    }

    void IndependentSet::setNewNextRightEndpoint(int where, int rightEndpoint)
    {
        _intervalIndexToNextRightEndpoint[where] = rightEndpoint;
    }

    void IndependentSet::assembleContainedIndependentSet(const cg::data_structures::Interval &interval)
    {
        auto next = _intervalIndexToNextRightEndpoint[interval.Left + 1];
        auto &containedSet = _intervalIndexToDirectlyContained[interval.Index];
        while (next != 0)
        {
            const auto &containedInterval = _intervals.getIntervalByRightEndpoint(next);
            containedSet.push_front(containedInterval.Index);
            next = _intervalIndexToNextRightEndpoint[next];
        }
    }

    std::vector<cg::data_structures::Interval> IndependentSet::buildIndependentSet()
    {
        std::vector<cg::data_structures::Interval> intervalsInMis; 
        intervalsInMis.reserve(_intervals.size);

        std::vector<cg::data_structures::Interval> pendingIntervals;
        pendingIntervals.reserve(_intervals.size);
        auto next = _intervalIndexToNextRightEndpoint[0];
        while(next != 0)
        {
            const auto& interval = _intervals.getIntervalByRightEndpoint(next);
            pendingIntervals.push_back(interval);
            while(!pendingIntervals.empty())
            {
                const auto& newInterval = pendingIntervals.back();
                pendingIntervals.pop_back();
                intervalsInMis.push_back(newInterval);
                const auto& allContained = _intervalIndexToDirectlyContained[newInterval.Index];
                for(auto idx : allContained)
                {   
                    const auto& c = _intervals.getIntervalByIndex(idx);
                    pendingIntervals.push_back(c);
                }
            }
            next = _intervalIndexToNextRightEndpoint[next];
        }

        return intervalsInMis;
    }
}