#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>
#include <tuple>

#include <algorithm>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/monotone_seq.h"
#include "mis/interval_store.h"
#include "utils/counters.h"

#include "mis/distinct/lazy_output_sensitive.h"



#include <iostream>


namespace cg::mis::distinct
{
    bool LazyOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, int leftLimit, std::map<int, PendingUpdate> &pendingUpdates, ImplicitIndependentSet& independentSet, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
       
        std::vector<int> mis(intervals.end+1, 0);

        std::vector<cg::mis::MonotoneSeq::Range> changed;

        while (!pendingUpdates.empty())
        {

            counts.Increment(Counts::StackOuterLoop);

            auto it = std::prev(pendingUpdates.end());
            auto newUpdate = it->second;
            auto currentInterval = newUpdate.interval;
            auto currentIntervalCandidate = 1 + CMIS[currentInterval.Index] + MIS.get(currentInterval.Right + 1);//newUpdate.candidate;

            std::cout << std::format("GOT {}, candiate = {}, MIS at left = {}", currentInterval, currentIntervalCandidate, MIS.get(currentInterval.Left)) << std::endl;
            if(currentInterval.Left < leftLimit)
            {
                std::cout << std::format("WILL NOT PROCESS INTERVAL {}, left limit is {}", currentInterval, leftLimit) << std::endl;
                break;
            }

            pendingUpdates.erase(it);

            if(currentIntervalCandidate <= MIS.get(currentInterval.Left))
            {
                std::cout << std::format("CURRENT INTERVAL CANDIDATE TOO SMALL {}, MIS[{}]={}, MIS[{}] = {}", currentIntervalCandidate, currentInterval.Left, MIS.get(currentInterval.Left), currentInterval.Right + 1, MIS.get(currentInterval.Right + 1)) << std::endl;
                continue;
            }


            cg::mis::MonotoneSeq::Range r = MIS.set(currentInterval.Left, currentIntervalCandidate);
            changed.push_back(r);
            std::cout << std::format(" --- SET AT {} to {}, Range = {} {}", currentInterval.Left, currentIntervalCandidate, r.changeStartInclusive, r.changeEndExclusive) << std::endl;
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
  
            //independentSet.setRange(r.left, r.right - 1, currentInterval);
 
            auto representativeMIS = currentIntervalCandidate;
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.changeEndExclusive);


            while(maybeInterval) 
            {
                counts.Increment(Counts::StackInnerLoop);
                auto interval = maybeInterval.value();
                if(interval.Left >= r.changeStartInclusive)
                {
                    std::cout << std::format("         INSPECTING: {} -- LEFT IS AFTER change-start {}", interval, r.changeStartInclusive) << std::endl;
                    maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
                    continue;
                }
                if(interval.Right < r.changeStartInclusive)
                {
                    std::cout << std::format("         INSPECTING: {} -- RIGHT IS BEFORE change-start {}", interval, r.changeStartInclusive) << std::endl;
                    break;
                }

                auto candidate = interval.Weight + CMIS[interval.Index] + representativeMIS;
                if (candidate > maxAllowedMIS)
                {
                    return false;
                }
                if (candidate > MIS.get(interval.Left))
                {
                    std::cout << std::format("         INSPECTING: {} -- CANDIDATE MIS {} IS LARGER than {}", interval, candidate, MIS.get(interval.Left))  << std::endl;
                    auto largerLeft = pendingUpdates.upper_bound(interval.Left);
                     
                    if(largerLeft != pendingUpdates.end() && MIS.get(largerLeft->second.interval.Left) == MIS.get(interval.Left))
                    {
                        std::cout << std::format("         NOT PUSHING {} there is a larger left interval {}", interval, largerLeft->second.interval)  << std::endl;
                    }
                    else 
                    {
                        if (pendingUpdates.size() > 1)
                        {
                            auto smallerLeft = pendingUpdates.upper_bound(interval.Left);

                            if (smallerLeft != pendingUpdates.begin())
                            {
                                --smallerLeft;
                                if (MIS.get(smallerLeft->second.interval.Left) == MIS.get(interval.Left))
                                {
                                    std::cout << std::format("         ERASING {} it is a SMALLER LEFT than this interval interval {}", smallerLeft->second.interval, interval) << std::endl;
                                    pendingUpdates.erase(smallerLeft);
                                }
                            }
                        }

                        std::cout << std::format("        ---- PUSHING {}", interval) << std::endl;
                        auto [it2, wasInserted] = pendingUpdates.emplace(interval.Left, PendingUpdate{interval,candidate});
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
            std::cout << std::format("[{}, {}], ", r.changeEndExclusive-1, r.changeStartInclusive);
        }
        std::cout << std::endl;
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> LazyOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::map<int, PendingUpdate> pendingUpdates;
        std::vector<int> CMIS(intervals.size);
        cg::mis::MonotoneSeq MIS(intervals.end+1);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            counts.Increment(Counts::IntervalOuterLoop);
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                std::cout << std::format("ENCOUNTERED INTERVAL {}, left limit is {}", interval, interval.Left + 1) << std::endl;

                if (!tryUpdate(intervals, interval.Left + 1, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts))
                {
                    return std::nullopt;
                }
                auto cmis = MIS.get(interval.Left + 1);
                CMIS[interval.Index] = cmis;
                std::cout << std::format("CMIS[{}] = {}", interval, CMIS[interval.Index]) << std::endl;

                independentSet.assembleContainedIndependentSet(interval);

                pendingUpdates.emplace(interval.Left, PendingUpdate{interval, 1 + cmis});
            }
        }
        if (!tryUpdate(intervals, 0, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts))
        {
            return std::nullopt;
        }

        std::cout << "IntervalOuter = " << counts.Get(Counts::IntervalOuterLoop) << std::endl;
        std::cout << "StackOuter = " << counts.Get(Counts::StackOuterLoop) << std::endl;
        std::cout << "StackInner = " << counts.Get(Counts::StackInnerLoop) << std::endl;
        std::cout << "StackInner / (alpha*n) = " << counts.Get(Counts::StackInnerLoop) / (float) (MIS.get(0) * intervals.size) << std::endl;
        std::cout << "StackInner / (alpha*alpha) = " << counts.Get(Counts::StackInnerLoop) / (float) (MIS.get(0) * MIS.get(0)) << std::endl;


        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));

        return intervalsInMis;
    }
}
