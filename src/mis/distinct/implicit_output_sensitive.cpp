#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/distinct/implicit_output_sensitive.h"

#include <iostream>

namespace cg::mis::distinct
{
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        pendingUpdates.push(newInterval.Left);
        independentSet.addInterval(newInterval);

        while (!pendingUpdates.empty())
        {
            auto updatedIndex = pendingUpdates.top();
            pendingUpdates.pop();

            MonotoneSeq::Range r = MIS.increment(updatedIndex);
            if(r.right - 1 > r.left)
            {
                independentSet.setSameNextInterval(r.left, r.right - 1);
            }
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.right);
            while(maybeInterval) // This iterates O(min(d, alpha)) times, taking O(log^2 n) time per iteration.
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
                    independentSet.addInterval(interval);
                }
                /*else
                {
                    auto currentLeft = r.left;
                    auto currentRight = interval.Right;
                    while(true)
                    {
                        auto mid = (currentLeft + currentRight) >> 1;
                        auto midPred = intervals.tryGetRightEndpointPredecessorInterval(mid);
                        auto foundNewCMISWithRightEndpointInRange = midPred.has_value() && midPred.value().Right >= r.left && CMIS[midPred.value().Index] > CMIS[interval.Index] && MIS.get(midPred.value().Left) == thisRegionValue;
                        if(foundNewCMISWithRightEndpointInRange)
                        {
                            currentLeft = mid; 
                        }
                        else
                        {
                            currentRight = mid;
                        }
                    }
                    auto foundNewCMISInThisRegion = false;
                    if(foundNewCMISInThisRegion)
                    {
                        // If we found a new CMIS in this region, set maybeInterval to that interval,
                    }
                    else
                    {
                        // Set maybeInterval to the first interval in a new region (one-sided bsearch)...
                    }
                }*/

                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> ImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS)
    {
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;
        MonotoneSeq MIS(intervals.end);

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
