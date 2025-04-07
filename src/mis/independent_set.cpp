#include "data_structures/interval.h"
#include "utils/interval_rep_utils.h"

#include "mis/independent_set.h"

namespace cg::mis
{
    IndependentSet::IndependentSet(int maxNumIntervals)
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

    std::vector<cg::data_structures::Interval> IndependentSet::buildIndependentSet()
    {
        const auto maxNumIntervals = _intervalIndexToDirectlyContained.size();
        std::vector<cg::data_structures::Interval> intervalsInMis; 
        intervalsInMis.reserve(maxNumIntervals);

        std::vector<cg::data_structures::Interval> pendingIntervals;
        pendingIntervals.reserve(maxNumIntervals);
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
        cg::utils::verifyNoOverlaps(intervalsInMis);
        return intervalsInMis;
    }
}