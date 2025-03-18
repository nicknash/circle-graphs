#include <vector>
#include <list>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"

#include "mis/valiente.h"

namespace cg::mis
{
    std::vector<cg::data_structures::Interval> Valiente::computeMIS(const cg::data_structures::SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.end + 1, 0);
        std::vector<int> CMIS(intervals.size, 0);

        std::vector<int> nextRightEndpoint(intervals.end + 1, 0);
        std::vector<std::list<int>> intervalIndexToDirectlyContained(intervals.end); 

        for(auto i = 0; i < intervals.end; ++i)
        {
            auto maybeOuterInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeOuterInterval)
            {
                auto outerInterval = maybeOuterInterval.value();
                for(auto j = outerInterval.Right; j > outerInterval.Left; --j)
                {
                    auto maybeInnerInterval = intervals.tryGetIntervalByLeftEndpoint(j);
                    MIS[j] = MIS[j + 1];
                    nextRightEndpoint[j] = nextRightEndpoint[j + 1];
                    if(maybeInnerInterval)
                    {
                        auto innerInterval = maybeInnerInterval.value();
                        auto candidate = MIS[innerInterval.Right + 1] + CMIS[innerInterval.Index];
                        if(innerInterval.Right < outerInterval.Right && // Strictly speaking, this bounds check could be removed because CMIS and MIS on the previous line
                                                                        // will both be zero when it is false, but it's a bit confusing to write the code that way. 
                           candidate > MIS[j + 1])
                        {
                            MIS[j] = candidate;
                            nextRightEndpoint[j] = innerInterval.Right;
                        }
                    }
                }
                CMIS[outerInterval.Index] = 1 + MIS[outerInterval.Left + 1];
                auto next = nextRightEndpoint[outerInterval.Left + 1];
                auto& containedSet = intervalIndexToDirectlyContained[outerInterval.Index];
                while(next != 0)
                {
                    const auto& containedInterval = intervals.getIntervalByRightEndpoint(next);
                    containedSet.push_front(containedInterval.Index);
                    next = nextRightEndpoint[next];
                }
            }
        }

        
        for(auto i = intervals.end - 1; i >= 0; --i)
        {
            auto maybeInterval = intervals.tryGetIntervalByLeftEndpoint(i);
            nextRightEndpoint[i] = nextRightEndpoint[i + 1];
            
            MIS[i] = MIS[i + 1];
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto candidate = MIS[interval.Right + 1] + CMIS[interval.Index];
                if(candidate > MIS[i + 1])
                {
                    MIS[i] = candidate;
                    nextRightEndpoint[i] = interval.Right;
                }
            }
        }

        std::vector<cg::data_structures::Interval> intervalsInMis; 
        intervalsInMis.reserve(intervals.size);

        std::vector<cg::data_structures::Interval> pendingIntervals;
        pendingIntervals.reserve(intervals.size);

       auto next = nextRightEndpoint[0];
        while(next != 0)
        {
            const auto& interval = intervals.getIntervalByRightEndpoint(next);
            pendingIntervals.push_back(interval);
            while(!pendingIntervals.empty())
            {
                const auto& newInterval = pendingIntervals.back();
                pendingIntervals.pop_back();
                intervalsInMis.push_back(newInterval);
                const auto& allContained = intervalIndexToDirectlyContained[newInterval.Index];
                for(auto idx : allContained)
                {   
                    const auto& c = intervals.getIntervalByIndex(idx);
                    pendingIntervals.push_back(c);
                }
            }
            next = nextRightEndpoint[next];
        }
        return intervalsInMis;
    }
}
