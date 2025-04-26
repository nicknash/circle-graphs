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
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        pendingUpdates.push(newInterval);
        std::cout << std::format(" **** NEW INTERVAL {} DETECTED **** ", newInterval) << std::endl;

        std::vector<int> mis(intervals.end+1, 0);
        while (!pendingUpdates.empty())
        {
            auto currentInterval = pendingUpdates.top();
            pendingUpdates.pop();

            cg::mis::MonotoneSeq::Range r = MIS.increment(currentInterval.Left);
            std::cout << std::format("POPPED {}. INCREMENTED REGION {} {} {}", currentInterval, r.left, r.changePoint, r.right) << std::endl;
            for(int i = 0; i < mis.size(); ++i)
            {
                std::cout << i << " ";
            }
            std::cout << std::endl;
            MIS.copyTo(mis);
            
            for(int i = 0; i < mis.size(); ++i)
            {
                if(i > 9 && mis[i] < 10)
                {
                    std::cout << " ";
                }
                std::cout << mis[i] << " ";
            }
            std::cout << std::endl << std::endl;
            independentSet.setRange(r.left, r.right - 1, currentInterval);
 
            auto representativeMIS = MIS.get(r.changePoint);
      
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.right);
            while(maybeInterval) 
            {
                auto interval = maybeInterval.value();
                if(interval.Left >= r.changePoint)
                {
                    std::cout << std::format("         INSPECTING: {} -- LEFT IS AFTER CHANGEPOINT {}", interval, r.changePoint) << std::endl;
                    continue;
                }
                if(interval.Right < r.changePoint)
                {
                    std::cout << std::format("         INSPECTING: {} -- RIGHT IS BEFORE CHANGEPOINT {}", interval, r.changePoint) << std::endl;
                    break;
                }

                auto candidate = interval.Weight + CMIS[interval.Index] + representativeMIS;
                if (candidate > maxAllowedMIS)
                {
                    return false;
                }
                if (candidate > MIS.get(interval.Left))
                {
                    std::cout << std::format("         INSPECTING: {} -- CANDIDATE MIS {} IS LARGER than {} --- PUSHING!", interval, candidate, MIS.get(interval.Left))  << std::endl;
                    pendingUpdates.push(interval);
                }
                else
                {
                    std::cout << std::format("         INSPECTING: {} -- CANDIDATE MIS {} not larger than {}", interval, candidate, MIS.get(interval.Left)) << std::endl;

                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> ImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS)
    {
        std::stack<cg::data_structures::Interval> pendingUpdates;
        std::vector<int> CMIS(intervals.size);
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
                CMIS[interval.Index] = cmis;
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
