#include <vector>
#include <list>
#include <algorithm>

#include "data_structures/shared_interval_model.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"
#include "utils/counters.h"

#include "mis/shared/valiente.h"

namespace cg::mis::shared
{
    std::optional<cg::data_structures::Interval> Valiente::getMaxInterval(std::span<const cg::data_structures::Interval> intervals, int maxRightEndpoint, std::vector<int> &MIS, std::vector<int>& CMIS, cg::utils::Counters<Counts>& counts)
    {
        std::optional<cg::data_structures::Interval> maxInterval;
        
        auto max = 0;
        for(const auto& interval : intervals)
        {
            counts.Increment(Counts::InnerMaxLoop);
            if(interval.Right < maxRightEndpoint)
            {
                auto candidateMax = CMIS[interval.Index] + MIS[interval.Right + 1];
                if(candidateMax > max)
                {
                    max = candidateMax;
                    maxInterval.emplace(interval);
                }
            }
        }
        return maxInterval;
    }

    std::vector<cg::data_structures::Interval> Valiente::computeMIS(const cg::data_structures::SharedIntervalModel &intervals, cg::utils::Counters<Counts> &counts)
    {
        std::vector<int> MIS(1 + intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        cg::mis::IndependentSet independentSet(intervals.size);

        for (auto right = 1; right < intervals.end + 1; ++right)
        {
            const auto &rightIntervals = intervals.getAllIntervalsWithRightEndpoint(right - 1);
            if (rightIntervals.size() > 0)
            {
                const auto &longest = rightIntervals[0];
                for (auto here = longest.Right - 1; here > longest.Left; --here)
                {
                    counts.Increment(Counts::InnerLoop);
                    const auto &intervalsWithLeftEndpointHere = intervals.getAllIntervalsWithLeftEndpoint(here);
                    const auto &maybeMaxInterval = getMaxInterval(intervalsWithLeftEndpointHere, right - 1, MIS, CMIS, counts);
                    independentSet.setSameNextInterval(here);
                    MIS[here] = MIS[here + 1];
                    if (maybeMaxInterval)
                    {
                        const auto &maxInterval = maybeMaxInterval.value();
                        const auto candidate = CMIS[maxInterval.Index] + MIS[maxInterval.Right + 1];
                        if (candidate > MIS[here + 1])
                        {
                            MIS[here] = candidate;
                            independentSet.setNewNextInterval(here, maxInterval);
                        }
                    }
                }
                for(const auto& interval : rightIntervals)
                {
                    CMIS[interval.Index] = interval.Weight + MIS[interval.Left + 1];
                    independentSet.assembleContainedIndependentSet(interval);
                }
            }
        }
        for(auto left = intervals.end - 1; left >= 0; --left)
        {
            auto leftIntervals = intervals.getAllIntervalsWithLeftEndpoint(left);
            independentSet.setSameNextInterval(left);
            MIS[left] = MIS[left + 1];
            
            for(auto interval : leftIntervals)
            {
                auto candidate = MIS[interval.Right + 1] + CMIS[interval.Index];
                if(candidate > MIS[left + 1])
                {
                    MIS[left] = candidate;
                    independentSet.setNewNextInterval(left, interval);
                }
            }
        }
        auto intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}
