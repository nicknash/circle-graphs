#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>
#include <cmath>

#include <algorithm>

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/unit_monotone_seq.h"
#include "utils/counters.h"

#include "mis/distinct/implicit_output_sensitive.h"


namespace cg::mis::distinct
{
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, std::map<int, cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::UnitMonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        pendingUpdates.emplace(newInterval.Left, newInterval);

        while (!pendingUpdates.empty())
        {
            counts.Increment(Counts::StackOuterLoop);

            auto it = std::prev(pendingUpdates.end());
            auto currentInterval = it->second;
            pendingUpdates.erase(it);

            cg::mis::UnitMonotoneSeq::Range r = MIS.increment(currentInterval.Left);
            independentSet.setRange(r.left, r.right - 1, currentInterval);

            auto representativeMIS = MIS.get(r.changePoint);
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.right);
            while(maybeInterval)
            {
                auto interval = maybeInterval.value();
                if(interval.Left >= r.changePoint)
                {
                    maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
                    continue;
                }
                if(interval.Right < r.changePoint)
                {
                    break;
                }
                counts.Increment(Counts::StackInnerLoop);

                auto candidate = interval.Weight + CMIS[interval.Index] + representativeMIS;
                if (candidate > maxAllowedMIS)
                {
                    return false;
                }
                if (candidate > MIS.get(interval.Left))
                {
                    auto largerLeft = pendingUpdates.upper_bound(interval.Left);

                    if(!(largerLeft != pendingUpdates.end() && MIS.get(largerLeft->second.Left) == MIS.get(interval.Left)))
                    {
                        if (pendingUpdates.size() > 1)
                        {
                            auto smallerLeft = pendingUpdates.upper_bound(interval.Left);

                            if (smallerLeft != pendingUpdates.begin())
                            {
                                --smallerLeft;
                                if (MIS.get(smallerLeft->second.Left) == MIS.get(interval.Left))
                                {
                                    pendingUpdates.erase(smallerLeft);
                                }
                            }
                        }

                        pendingUpdates.emplace(interval.Left, interval);
                    }
                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);

            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> ImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::map<int, cg::data_structures::Interval> pendingUpdates;
        std::vector<int> CMIS(intervals.size);
        cg::mis::UnitMonotoneSeq MIS(intervals.end);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            counts.Increment(Counts::IntervalOuterLoop);
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto cmis = MIS.get(interval.Left + 1);
                CMIS[interval.Index] = cmis;
                independentSet.assembleContainedIndependentSet(interval);
                if (!tryUpdate(intervals, pendingUpdates, independentSet, interval, MIS, CMIS, maxAllowedMIS, counts))
                {
                    return std::nullopt;
                }
            }
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));

        return intervalsInMis;
    }
}
