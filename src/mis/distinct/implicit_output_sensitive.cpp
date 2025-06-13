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
#include "utils/counters.h"

#include "mis/distinct/implicit_output_sensitive.h"



#include <iostream>


namespace cg::mis::distinct
{
    bool ImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, std::map<int, cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::UnitMonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        pendingUpdates.emplace(newInterval.Left, newInterval);
        std::cout << std::format(" **** NEW INTERVAL {} DETECTED **** ", newInterval) << std::endl;

        std::vector<int> mis(intervals.end+1, 0);

        std::vector<cg::mis::UnitMonotoneSeq::Range> changed;

        while (!pendingUpdates.empty())
        {

            counts.Increment(Counts::StackOuterLoop);

            auto it = std::prev(pendingUpdates.end());
            auto currentInterval = it->second;
            pendingUpdates.erase(it);

            cg::mis::UnitMonotoneSeq::Range r = MIS.increment(currentInterval.Left);
            changed.push_back(r);
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
            /*while(maybeInterval)
            {
                auto interval = maybeInterval.value();

                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
            }*/
        

            while(maybeInterval) 
            {
//                counts.Increment(Counts::StackInnerLoop);
                auto interval = maybeInterval.value();
                if(interval.Left >= r.changePoint)
                {
                    std::cout << std::format("         INSPECTING: {} -- LEFT IS AFTER CHANGEPOINT {}", interval, r.changePoint) << std::endl;
                    maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
                    continue;
                }
                if(interval.Right < r.changePoint)
                {
                    std::cout << std::format("         INSPECTING: {} -- RIGHT IS BEFORE CHANGEPOINT {}", interval, r.changePoint) << std::endl;
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
                    std::cout << std::format("         INSPECTING: {} -- CANDIDATE MIS {} IS LARGER than {}", interval, candidate, MIS.get(interval.Left))  << std::endl;
                    auto largerLeft = pendingUpdates.upper_bound(interval.Left);
                     
                    if(largerLeft != pendingUpdates.end() && MIS.get(largerLeft->second.Left) == MIS.get(interval.Left))
                    {
                        std::cout << std::format("         NOT PUSHING {} there is a larger left interval {}", interval, largerLeft->second)  << std::endl;
                    }
                    else 
                    {
                        if (pendingUpdates.size() > 1)
                        {
                            auto smallerLeft = pendingUpdates.upper_bound(interval.Left);

                            if (smallerLeft != pendingUpdates.begin())
                            {
                                --smallerLeft;
                                if (MIS.get(smallerLeft->second.Left) == MIS.get(interval.Left))
                                {
                                    std::cout << std::format("         ERASING {} it is a SMALLER LEFT than this interval interval {}", smallerLeft->second, interval) << std::endl;
                                    pendingUpdates.erase(smallerLeft);
                                }
                            }
                        }

                        std::cout << std::format("        ---- PUSHING {}", interval) << std::endl;
                        auto [it2, wasInserted] = pendingUpdates.emplace(interval.Left, interval);
                        if (!wasInserted)
                        {
                            std::cout << std::format("DROPPED {}", interval) << std::endl;
                        }
                    }
                }
                else
                {

                    std::cout << std::format("         INSPECTING: {} -- CANDIDATE MIS {} not larger than {}", interval, candidate, MIS.get(interval.Left)) << std::endl;

                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);

            }
        }
        std::cout << "CHANGED REGIONS" << std::endl;
        for(auto r : changed)
        {
            std::cout << std::format("[{}, {}], ", r.right-1, r.changePoint);
        }
        std::cout << std::endl;
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
