#include <vector>
#include <list>
#include <algorithm>

#include "data_structures/shared_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"
#include "utils/counters.h"

#include "mis/shared/naive.h"

#include <iostream>

namespace cg::mis::shared
{
    std::optional<cg::data_structures::Interval> Naive::getMaxInterval(std::span<const cg::data_structures::Interval> intervals, int maxRightEndpoint, std::vector<int> &MIS, std::vector<int>& CMIS, cg::utils::Counters<Counts>& counts, std::vector<int>& indexToLastWinner)
    {
        std::optional<cg::data_structures::Interval> maxInterval;
        
        auto max = 0;
        auto sawAll = true;
        for(const auto& interval : intervals)
        {
            counts.Increment(Counts::InnerMaxLoop);
            if(interval.Right < maxRightEndpoint)
            {
                auto candidateMax = interval.Weight + CMIS[interval.Index] + MIS[interval.Right + 1];
                if(candidateMax > max)
                {
                    max = candidateMax;
                    maxInterval.emplace(interval);
                }
            }
            else
            {
                sawAll = false;
            }
        }
        if(sawAll && maxInterval.has_value())
        {
            auto mi = maxInterval.value();
            auto lastWinner = indexToLastWinner[mi.Left];
            if(lastWinner != -1 && lastWinner != mi.Index)
            {

                auto x = indexToLastWinner[mi.Index];
                //std::cout << std::format("MAX INTERVAL changed at {} from {} to {} (newlength = {})", mi.Left, lastWinner, mi.Index, mi.length()) << std::endl;
            }
            indexToLastWinner[mi.Left] = mi.Index;
        }
        return maxInterval;
    }


    std::vector<cg::data_structures::Interval> Naive::computeMIS(const cg::data_structures::SharedIntervalRep& intervals, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> MIS(1 + intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::vector<int> indexToLastWinner(1 + intervals.end, -1);
        cg::mis::IndependentSet independentSet(intervals.size);

        for(auto right = 1; right < intervals.end + 1; ++right)
        {
            for(auto here = right - 1; here >= 0; --here)
            {
                counts.Increment(Counts::InnerLoop);
                const auto& intervalsWithLeftEndpointHere = intervals.getAllIntervalsWithLeftEndpoint(here);
                
                for(const auto& interval : intervalsWithLeftEndpointHere)
                {
                    if(interval.Right == right)
                    {
                        CMIS[interval.Index] = MIS[here + 1];
                        independentSet.assembleContainedIndependentSet(interval);
                        break;
                    }
                }

                const auto& maybeMaxInterval = getMaxInterval(intervalsWithLeftEndpointHere, right, MIS, CMIS, counts, indexToLastWinner);
                independentSet.setSameNextInterval(here);
                MIS[here] = MIS[here + 1];
                if(maybeMaxInterval)
                {
                    const auto& maxInterval = maybeMaxInterval.value();
                    const auto candidate = maxInterval.Weight + CMIS[maxInterval.Index] + MIS[maxInterval.Right + 1];
                    if(candidate > MIS[here + 1])
                    {
                        MIS[here] = candidate;
                        independentSet.setNewNextInterval(here, maxInterval);
                    }
                }
            }
            /*for(auto x = 0; x < right; ++x)
            {
                std::cout << MIS[x] << " ";
            }
            std::cout << std::endl;*/

        }

        auto intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}

