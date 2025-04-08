#include <vector>
#include <list>
#include <algorithm>

#include "data_structures/shared_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/shared/naive.h"

namespace cg::mis::shared
{
    std::optional<cg::data_structures::Interval> Naive::getMaxInterval(std::span<const cg::data_structures::Interval> intervals, int maxRightEndpoint, std::vector<int> &MIS, std::vector<int>& CMIS)
    {
        std::optional<cg::data_structures::Interval> maxInterval;
        
        auto max = 0;
        for(const auto& interval : intervals)
        {
            if(interval.Right <= maxRightEndpoint)
            {
                auto candidateMax = 1 + CMIS[interval.Index] + MIS[interval.Right + 1];
                if(candidateMax > max)
                {
                    candidateMax = max;
                    maxInterval.emplace(interval);
                }
            }
        }
        return maxInterval;
    }


    std::vector<cg::data_structures::Interval> Naive::computeMIS(const cg::data_structures::SharedIntervalRep& intervals)
    {
        std::vector<int> MIS(1 + intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);

        cg::mis::IndependentSet independentSet(intervals.size);

        for(auto right = 1; right < intervals.end; ++right)
        {
            for(auto here = right - 1; here >= 0; --here)
            {
                const auto& intervalsWithLeftEndpointHere = intervals.getAllIntervalsWithLeftEndpoint(here);
                const auto& maybeMaxInterval = getMaxInterval(intervalsWithLeftEndpointHere, right, MIS, CMIS);
                independentSet.setSameNextInterval(here);
                MIS[here] = MIS[here + 1];
                if(maybeMaxInterval)
                {
                    const auto& maxInterval = maybeMaxInterval.value();
                    const auto candidate = 1 + CMIS[maxInterval.Index] + MIS[maxInterval.Right + 1];
                    if(candidate > MIS[here + 1])
                    {
                        MIS[here] = candidate;
                        independentSet.setNewNextInterval(here, maxInterval);
                    }
                }
                for(const auto& interval : intervalsWithLeftEndpointHere)
                {
                    if(interval.Right == right)
                    {
                        CMIS[interval.Index] = MIS[here + 1];
                        independentSet.assembleContainedIndependentSet(interval);
                        break;
                    }
                }
            }
        }

        auto intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}

