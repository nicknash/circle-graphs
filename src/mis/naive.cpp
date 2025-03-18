#include <vector>
#include <list>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"

#include "mis/naive.h"

namespace cg::mis
{
    std::vector<cg::data_structures::Interval> Naive::computeMIS(const cg::data_structures::SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.end + 1, 0);
        std::vector<int> CMIS(intervals.size, 0);
        
        std::vector<int> nextRightEndpoint(intervals.end + 1, 0);
        std::vector<std::list<int>> intervalIndexToDirectlyContained(intervals.end); 

        for(auto i = 0; i < intervals.end; ++i)
        {
            auto maybeIntervalRight = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeIntervalRight)
            {
                auto interval = maybeIntervalRight.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
            
                auto next = nextRightEndpoint[interval.Left + 1];
                auto& containedSet = intervalIndexToDirectlyContained[interval.Index];
                while(next != 0)
                {
                    const auto& containedInterval = intervals.getIntervalByRightEndpoint(next);
                    containedSet.push_front(containedInterval.Index);
                    next = nextRightEndpoint[next];
                }
            }
            for (auto j = i - 1; j >= 0; --j)
            {
                MIS[j] = MIS[j + 1];
                nextRightEndpoint[j] = nextRightEndpoint[j + 1];

                auto maybeIntervalLeft = intervals.tryGetIntervalByLeftEndpoint(j);
                if(maybeIntervalLeft)
                {
                    auto interval = maybeIntervalLeft.value();
                    if(interval.Right <= i)
                    {
                        auto candidate = 1 + CMIS[interval.Index] + MIS[interval.Right + 1];
                        if(candidate > MIS[j + 1])
                        {
                            nextRightEndpoint[j] = interval.Right;
                            MIS[j] = candidate;
                        }
                        else
                        {
                            MIS[j] = MIS[j + 1];
                            nextRightEndpoint[j] = nextRightEndpoint[j + 1];
                        }
                    }
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