#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>
#include <tuple>
#include <cmath>

#include <algorithm>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/monotone_seq.h"
#include "utils/counters.h"

#include "mis/distinct/lazy_output_sensitive.h"



#include <iostream>


namespace cg::mis::distinct
{
    bool LazyOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, int leftLimit, std::map<int, PendingUpdate> &pendingUpdates, ImplicitIndependentSet& independentSet, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
       
        std::vector<int> mis(intervals.end+1, 0);
        std::vector<int> prev(intervals.end+1, 0);


        std::vector<cg::mis::MonotoneSeq::Range> changed;

        //std::vector<cg::data_structures::Interval> 

        std::map<int, cg::data_structures::Interval> relevant;
        int maxSoFar = -1;

        while (!pendingUpdates.empty())
        {
            auto it = std::prev(pendingUpdates.end());
            auto newUpdate = it->second;
            auto currentInterval = newUpdate.interval;
            auto currentIntervalCandidate = currentInterval.Weight + CMIS[currentInterval.Index] + MIS.get(currentInterval.Right + 1);//newUpdate.candidate;
            if(currentIntervalCandidate <= maxSoFar)
            {
                //std::cout << "LESS THAN MAX SO FAR" << std::endl;
                //pendingUpdates.erase(it);
                //continue;
            }
            //std::cout << std::format("HERE: {}", currentInterval.Left) << std::endl;
            counts.Increment(Counts::StackOuterLoop);

            maxSoFar = currentIntervalCandidate;
            //std::cout << std::format("GOT {}, candiate = {}, MIS at left = {}", currentInterval, currentIntervalCandidate, MIS.get(currentInterval.Left)) << std::endl;
            if(currentInterval.Left < leftLimit)
            {
                //std::cout << std::format("WILL NOT PROCESS INTERVAL {}, left limit is {}", currentInterval, leftLimit) << std::endl;
                break;
            }

            pendingUpdates.erase(it);


            if(currentIntervalCandidate <= MIS.get(currentInterval.Left))
            {
                //std::cout << std::format("CURRENT INTERVAL CANDIDATE TOO SMALL {}, MIS[{}]={}, MIS[{}] = {}", currentIntervalCandidate, currentInterval.Left, MIS.get(currentInterval.Left), currentInterval.Right + 1, MIS.get(currentInterval.Right + 1)) << std::endl;
                continue;
            }


            cg::mis::MonotoneSeq::Range r = MIS.set(currentInterval.Left, currentIntervalCandidate);
            //changed.push_back(r);
            //std::cout << std::format(" --- SET AT {} to {}, Range = {} {}", currentInterval.Left, currentIntervalCandidate, r.changeStartInclusive, r.changeEndExclusive) << std::endl;
            /*for(int i = 0; i < mis.size(); ++i)
            {
                std::cout << i << " ";
            }
            std::cout << std::endl;*/
            MIS.copyTo(mis);
            std::cout << std::format("{}:\t", currentInterval);
            for(int i = 0; i < mis.size()-1; ++i)
            {
                if(i > 9 && mis[i] < 10)
                {
                   // std::cout << " ";
                }
                //std::cout << mis[i] - prev[i] << " ";
                std::cout << mis[i] - mis[i + 1] << " ";
                prev[i] = mis[i];
            }
            std::cout << std::endl;
            //std::cout << std::endl << std::endl;
  
            //independentSet.setRange(r.left, r.right - 1, currentInterval);
 
            int nextPending = -1;
            if(!pendingUpdates.empty())
            {
                 auto it2 = std::prev(pendingUpdates.end());
                nextPending = it2->first;
            }

            auto representativeMIS = currentIntervalCandidate;
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(currentInterval.Left); 
            while (maybeInterval)
            {
                auto interval = maybeInterval.value();
                if(interval.Right <= nextPending)
                {
                       counts.Increment(Counts::StackInnerLoop);
                    //pendingUpdates.emplace(interval.Left, PendingUpdate{interval, -1});

                    break;
                }
                
                if (interval.Left < r.changeStartInclusive && interval.Right >= r.changeStartInclusive)
                {
                    counts.Increment(Counts::StackInnerLoop);
                    pendingUpdates.emplace(interval.Left, PendingUpdate{interval, -1});

                    // NO HANG ON ... SHOULD ALSO BE CHECKING THE PREV PENDING! nextPending, just with a better condition...!
                    /*auto largerLeft = pendingUpdates.upper_bound(interval.Left);
                    if (largerLeft == pendingUpdates.end()) 
                    {
                        counts.Increment(Counts::StackInnerLoop);
                        pendingUpdates.emplace(interval.Left, PendingUpdate{interval, -1});
                    }
                    else
                    {
                        auto candidate1 = interval.Weight + CMIS[interval.Index] + MIS.get(interval.Right + 1);
                        auto interval2 = largerLeft->second.interval;
                        auto largerLeftCandidate = interval2.Weight + CMIS[interval2.Index] + MIS.get(interval2.Right + 1);
                        if(candidate1 > largerLeftCandidate)
                        {
                            counts.Increment(Counts::StackInnerLoop);
                            pendingUpdates.emplace(interval.Left, PendingUpdate{interval, -1});
                        }
                    }*/
                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
            }
        }
        /*std::cout << "CHANGED REGIONS" << std::endl;
        for(auto r : changed)
        {
            //std::cout << std::format("[{}, {}], ", r.changeEndExclusive-1, r.changeStartInclusive);
        }
        std::cout << std::endl;*/
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> LazyOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
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
                //std::cout << std::format("CMIS[{}] = {}", interval, CMIS[interval.Index]) << std::endl;

                independentSet.assembleContainedIndependentSet(interval);

                pendingUpdates.erase(interval.Left);
                pendingUpdates.emplace(interval.Left, PendingUpdate{interval, interval.Weight + cmis});
            }
        }
        //std::cout << " --- FINAL PENDING --- " << std::endl;
        if (!tryUpdate(intervals, 0, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts))
        {
            return std::nullopt;
        }

        std::cout << "IntervalOuter = " << counts.Get(Counts::IntervalOuterLoop) << std::endl;
        std::cout << "StackOuter = " << counts.Get(Counts::StackOuterLoop) << std::endl;
        std::cout << "StackInner = " << counts.Get(Counts::StackInnerLoop) << std::endl;
        std::cout << "StackOuter / (alpha*n) = " << counts.Get(Counts::StackOuterLoop) / (float) (MIS.get(0) * intervals.size) << std::endl;
        std::cout << "StackOuter / (n + alpha*alpha) = " << counts.Get(Counts::StackOuterLoop) / (float) (intervals.size + MIS.get(0) * MIS.get(0)) << std::endl;
        std::cout << "StackOuter / (n^1.5) = " << counts.Get(Counts::StackOuterLoop) / (float) (intervals.size * std::sqrt(intervals.size)) << std::endl;

        std::cout << "StackInner / (alpha*n) = " << counts.Get(Counts::StackInnerLoop) / (float) (MIS.get(0) * intervals.size) << std::endl;
        std::cout << "StackInner / (n + alpha*alpha) = " << counts.Get(Counts::StackInnerLoop) / (float) (intervals.size + MIS.get(0) * MIS.get(0)) << std::endl;
        std::cout << "StackInner / (n^1.5) = " << counts.Get(Counts::StackInnerLoop) / (float) (intervals.size * std::sqrt(intervals.size)) << std::endl;


        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));

        return intervalsInMis;
    }
}
