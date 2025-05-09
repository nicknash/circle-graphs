#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>
#include <cmath>

#include <algorithm>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/unit_monotone_seq.h"
#include "mis/interval_store.h"
#include "utils/counters.h"

#include "mis/distinct/simple_implicit_output_sensitive.h"



#include <iostream>


namespace cg::mis::distinct
{
    bool SimpleImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::UnitMonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        pendingUpdates.push(newInterval);
        while (!pendingUpdates.empty())
        {
            counts.Increment(Counts::StackOuterLoop);

            auto currentInterval = pendingUpdates.top();
            pendingUpdates.pop();

            cg::mis::UnitMonotoneSeq::Range r = MIS.increment(currentInterval.Left);
  
            independentSet.setRange(r.left, r.right - 1, currentInterval);
 
            auto representativeMIS = MIS.get(r.changePoint);
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.right);
  
            while(maybeInterval) 
            {
                counts.Increment(Counts::StackInnerLoop);
                auto interval = maybeInterval.value();
                if(interval.Right < r.changePoint)
                {
                    break;
                }
                if (interval.Left < r.changePoint)
                {
                    auto candidate = interval.Weight + CMIS[interval.Index] + representativeMIS;
                    if (candidate > maxAllowedMIS)
                    {
                        return false;
                    }
                    if (candidate > MIS.get(interval.Left))
                    {
                        pendingUpdates.push(interval);
                    }
                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);

            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> SimpleImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::stack<cg::data_structures::Interval> pendingUpdates;
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
        std::cout << "IntervalOuter = " << counts.Get(Counts::IntervalOuterLoop) << std::endl;
        std::cout << "StackOuter = " << counts.Get(Counts::StackOuterLoop) << std::endl;
        std::cout << "StackInner = " << counts.Get(Counts::StackInnerLoop) << std::endl;
        std::cout << "StackOuter / (alpha*n) = " << counts.Get(Counts::StackOuterLoop) / (float) (MIS.get(0) * intervals.size) << std::endl;
        std::cout << "StackOuter / (alpha*alpha) = " << counts.Get(Counts::StackOuterLoop) / (float) (MIS.get(0) * MIS.get(0)) << std::endl;
        std::cout << "StackOuter / (n^1.5) = " << counts.Get(Counts::StackOuterLoop) / (float) (intervals.size * std::sqrt(intervals.size)) << std::endl;
        std::cout << "StackInner / (alpha*n) = " << counts.Get(Counts::StackInnerLoop) / (float) (MIS.get(0) * intervals.size) << std::endl;
        std::cout << "StackInner / (alpha*alpha) = " << counts.Get(Counts::StackInnerLoop) / (float) (MIS.get(0) * MIS.get(0)) << std::endl;
        std::cout << "StackInner / (n^1.5) = " << counts.Get(Counts::StackInnerLoop) / (float) (intervals.size * std::sqrt(intervals.size)) << std::endl;

        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));

        return intervalsInMis;
    }
}
