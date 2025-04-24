#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/monotone_seq.h"
#include "mis/interval_store.h"

#include "mis/distinct/implicit_output_sensitive.h"

#include <iostream>


namespace cg::mis::distinct
{
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::MonotoneSeq &MIS, std::map<int, cg::mis::IntervalStore> &cmisToIntervalStore, int maxAllowedMIS)
    {
        pendingUpdates.push(newInterval);

        while (!pendingUpdates.empty())
        {
            auto currentInterval = pendingUpdates.top();
            pendingUpdates.pop();

            cg::mis::MonotoneSeq::Range r = MIS.increment(currentInterval.Left);
            
            independentSet.setRange(r.left, r.right - 1, currentInterval);
 
            auto representativeMIS = MIS.get(r.changePoint);
            for(auto it = cmisToIntervalStore.begin(); it != cmisToIntervalStore.end(); ++it)
            {
                auto [cmis, intervalStore] = *it;
                auto maybeInterval = intervalStore.tryGetRelevantInterval(r.changePoint, r.right); 
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
        std::map<int, IntervalStore> cmisToIntervals;
        cg::mis::MonotoneSeq MIS(intervals.end);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto cmis = MIS.get(interval.Left + 1);
                cmisToIntervals[cmis].addInterval(interval);
                independentSet.assembleContainedIndependentSet(interval);
                if (!tryUpdate(intervals, pendingUpdates, independentSet, interval, MIS, cmisToIntervals, maxAllowedMIS))
                {
                    return std::nullopt;
                }
            }
        }

        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));
        return intervalsInMis;
    }
}
