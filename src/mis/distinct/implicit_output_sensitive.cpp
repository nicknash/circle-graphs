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

namespace cg::mis::distinct
{
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        pendingUpdates.push(newInterval.Left);

        while (!pendingUpdates.empty())
        {
            auto updatedIndex = pendingUpdates.top();
            pendingUpdates.pop();

            cg::mis::MonotoneSeq::Range r = MIS.increment(updatedIndex);
            
            independentSet.setRange(r.left, r.right - 1, intervals.getIntervalByLeftEndpoint(updatedIndex));
 
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
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> ImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS)
    {
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;
        cg::mis::MonotoneSeq MIS(intervals.end);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS.get(interval.Left + 1);
                independentSet.assembleContainedIndependentSet(interval);
                if (!tryUpdate(intervals, pendingUpdates, independentSet, interval, MIS, CMIS, maxAllowedMIS))
                {
                    return std::nullopt;
                }
            }
        }

        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));
        return intervalsInMis;
    }
}
