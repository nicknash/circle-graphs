#include "data_structures/interval.h"
#include "utils/interval_rep_utils.h"

#include "mis/independent_set.h"

#include <iostream>

namespace cg::mis
{
    IndependentSet::IndependentSet(int maxNumIntervals) // should accept a max interval end-point really instead
    {
        _endpointToInterval.resize(2 * maxNumIntervals + 1);
        _intervalIndexToDirectlyContained.resize(maxNumIntervals);
    }

    void IndependentSet::setSameNextInterval(int where)
    {
        _endpointToInterval[where] = _endpointToInterval[where + 1];
    }

    void IndependentSet::setNewNextInterval(int where, const cg::data_structures::Interval& interval) 
    {
        _endpointToInterval[where] = interval;
    }

    // It's worth a quick explanation of the space complexity implied by calling assembleContainedIndependentSet for each of k intervals.
    // This results in, for each of the k intervals, the set of intervals directly contained in that interval being stored. Directly contained in an interval I, means intervals that are contained 
    // in I, but not in any other interval that is contained in I.
    //
    // This requires, for all the k intervals only O(k) space. The reason is that for any interval, it can only be directly contained in at most 2 other intervals (because, if there was a third
    // interval containing any interval, it would have to contain one of the other two, violating direct containment)
    void IndependentSet::assembleContainedIndependentSet(const cg::data_structures::Interval &interval)
    {
        std::optional<cg::data_structures::Interval> maybeNext = _endpointToInterval[interval.Left + 1];
        auto &containedSet = _intervalIndexToDirectlyContained[interval.Index];
        while (maybeNext)
        {
            const auto& next = maybeNext.value();
            containedSet.push_front(next);
            maybeNext = _endpointToInterval[next.Right + 1];
        }
    }

    std::vector<cg::data_structures::Interval> IndependentSet::buildIndependentSet(int expectedCardinality)
    {
        std::vector<cg::data_structures::Interval> intervalsInMis; 
        intervalsInMis.reserve(expectedCardinality);

        std::vector<cg::data_structures::Interval> pendingIntervals;
        pendingIntervals.reserve(expectedCardinality);
        auto maybeInterval = _endpointToInterval[0];
        while(maybeInterval)
        {
            const auto& interval = maybeInterval.value();
            pendingIntervals.push_back(interval);
            while(!pendingIntervals.empty())
            {
                const auto& newInterval = pendingIntervals.back();
                pendingIntervals.pop_back();
                intervalsInMis.push_back(newInterval);
                const auto& allContained = _intervalIndexToDirectlyContained[newInterval.Index];
                for(auto c : allContained)
                {   
                    pendingIntervals.push_back(c);
                }
            }
            maybeInterval = _endpointToInterval[interval.Right + 1];
        }
        if(intervalsInMis.size() != expectedCardinality)
        {
            throw std::runtime_error(std::format("Constructed independent set has cardinality {} but a maximum independent set is expected to have cardinality {}", 
            intervalsInMis.size(), expectedCardinality));
        }
        cg::utils::verifyNoOverlaps(intervalsInMis);
        return intervalsInMis;
    }
}