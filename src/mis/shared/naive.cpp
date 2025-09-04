#include <vector>

#include "data_structures/shared_interval_model.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"
#include "utils/counters.h"

#include "mis/shared/naive.h"

namespace cg::mis::shared
{
    std::optional<cg::data_structures::Interval> Naive::getMaxInterval(std::span<const cg::data_structures::Interval> intervals, int maxRightEndpoint, std::vector<int> &MIS, std::vector<int>& CMIS, cg::utils::Counters<Counts>& counts, std::vector<cg::data_structures::Interval>& indexToLastWinner)
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
            auto oldMax = lastWinner.Weight + CMIS[lastWinner.Index] + MIS[lastWinner.Right + 1];
            if(max == oldMax)
            {
                maxInterval = lastWinner;
                mi = lastWinner;
            }

            if(lastWinner.Weight != -1 && lastWinner.Index != mi.Index/* && mi.Left == 189*/)
            {

                auto x = indexToLastWinner[mi.Index];
            }
            indexToLastWinner[mi.Left] = mi;
        }
        return maxInterval;
    }


    std::vector<cg::data_structures::Interval> Naive::computeMIS(const cg::data_structures::SharedIntervalModel& intervals, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> MIS(1 + intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::vector<cg::data_structures::Interval> indexToLastWinner(intervals.size, cg::data_structures::Interval(0, 1, 0, -1));
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
                // We need a data structure of intervals with REP at 'here'
                // Now when 'here' increases, we need it so that we can compute maxInterval efficiently...
                // That is O(m) intervals, but it is WITHIN O(m) distinct CMIS+MIS sets.
                // So e.g. if these were separate sets, this would require something like O(d log d)
                // Now it only happens on a cell increase, so it's at worst k * alpha * d log d
                // 
                // MAYBE translate this to the valiente context....to see if any different 
            }
            // indexToLastWinner remains available for future iterations
        }

        auto intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}

