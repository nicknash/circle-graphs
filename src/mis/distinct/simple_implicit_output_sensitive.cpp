#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>
#include <cmath>
#include <format>

#include <algorithm>

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/unit_monotone_seq.h"
#include "utils/counters.h"

#include "mis/distinct/simple_implicit_output_sensitive.h"



#include <iostream>


namespace cg::mis::distinct
{
    std::vector<int> age;

    bool SimpleImplicitOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, std::stack<cg::data_structures::Interval> &pendingUpdates, ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &newInterval, cg::mis::UnitMonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> mis(intervals.end+1, 0);

        std::cout << std::format("Processing interval {}", newInterval) << std::endl;
        pendingUpdates.push(newInterval);
        while (!pendingUpdates.empty())
        {
            counts.Increment(Counts::StackOuterLoop);

            auto currentInterval = pendingUpdates.top();
            std::cout << std::format("\t Popped interval {}", currentInterval) << std::endl;
            pendingUpdates.pop();

            cg::mis::UnitMonotoneSeq::Range r = MIS.increment(currentInterval.Left);
  

            MIS.copyTo(mis);
            std::cout << "\t ";
            for(int i = 0; i < mis.size(); ++i)
            {
                std::cout << i << " ";
            }
            std::cout << std::endl;
            std::cout << "\t DAMIS: " << newInterval.Right << " ";

            for(int i = 0; i < mis.size(); ++i)
            {
                if(i > 9 && mis[i] < 10)
                {
                    std::cout << " ";
                }
                std::cout << mis[i] << " ";
            }
            std::cout << std::endl;

            for(int i = 0; i < age.size(); ++i)
            {
                if(i > 9 && age[i] < 10)
                {
                    std::cout << " ";
                }
                std::cout << age[i] << " ";
            }
            
            independentSet.setRange(r.left, r.right - 1, currentInterval);
            

            std::cout << std::endl;
            std::cout << "\t Current MIS is: ";
            auto intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));
            for(auto q : intervalsInMis)
            {
                std::cout << std::format("{}", q) << ", ";
            }
            std::cout << std::endl;

            // Maintain 'alive' here ? 
            // If rep updated but NOT left, then its alive
            // figure out if there is a way to maintain this.
 
            auto representativeMIS = MIS.get(r.changePoint);
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(r.right);
            std::cout << std::format("\t Range is Left={},Right={},ChangedPoint={}", r.left, r.right, r.changePoint) << std::endl;
            while(maybeInterval) 
            {
                auto interval = maybeInterval.value();
                if(interval.Right < r.changePoint)
                {
                    break;
                }
                if (interval.Left < r.changePoint)
                {
                    counts.Increment(Counts::StackInnerLoop);
                    auto candidate = interval.Weight + CMIS[interval.Index] + representativeMIS;
                    if (candidate > maxAllowedMIS)
                    {
                        return false;
                    }
                    if (candidate > MIS.get(interval.Left))
                    {
                        std::cout << std::format("\t Encountered interval {} in range and PUSHED. LeftAge = {}, RightAge = {}", interval, age[interval.Left], age[interval.Right]) << std::endl;
                        pendingUpdates.push(interval);
                    }
                    else
                    {
                        std::cout << std::format("\t Encountered interval {} in range and DID NOT PUSH.  LeftAge = {}, RightAge = {}", interval, age[interval.Left], age[interval.Right]) << std::endl;
                    }

                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);

            }

            for(int i = r.changePoint; i < r.right; ++i)
            {
                age[i] = newInterval.Right;
            }

        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> SimpleImplicitOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::stack<cg::data_structures::Interval> pendingUpdates;
        std::vector<int> CMIS(intervals.size);
        cg::mis::UnitMonotoneSeq MIS(intervals.end);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

        for(int i = 0; i < intervals.end+1; ++i)
        {
            age.push_back(0);
        }

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
