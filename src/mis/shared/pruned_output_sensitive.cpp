#include <vector>
#include <stack>
#include <list>
#include <ranges>

#include "data_structures/shared_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/shared/pruned_output_sensitive.h"

#include "utils/counters.h"

namespace cg::mis::shared
{
    void PrunedOutputSensitive::updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue)
    {
        if(MIS[indexToUpdate] >= newMisValue)
        {
            throw std::runtime_error(std::format("MIS[{}] = {}, attempting to update it to {}, but the new value should be strictly greater.", indexToUpdate, MIS[indexToUpdate], newMisValue));
        }
        MIS[indexToUpdate] = newMisValue;
        pendingUpdates.push(indexToUpdate);
    }

    bool PrunedOutputSensitive::tryUpdate(const cg::data_structures::SharedIntervalModel &intervals, std::stack<int> &pendingUpdates, IndependentSet &independentSet, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts, std::vector<std::list<cg::data_structures::Interval>>& indexToRelevantIntervals)
    {
        while (!pendingUpdates.empty())
        {
            counts.Increment(Counts::StackOuterLoop);
            const auto updatedIndex = pendingUpdates.top();
            pendingUpdates.pop();
            const auto leftNeighbour = updatedIndex - 1;
            if (leftNeighbour < 0)
            {
                continue;
            }
            if (MIS[updatedIndex] > MIS[leftNeighbour])
            {
                updateAt(pendingUpdates, MIS, leftNeighbour, MIS[updatedIndex]);
                independentSet.setSameNextInterval(leftNeighbour);
            }
            const auto &relevantIntervals = indexToRelevantIntervals[leftNeighbour];
            for (auto interval : relevantIntervals)
            {
                counts.Increment(Counts::StackInnerLoop);
                const auto candidate = interval.Weight + CMIS[interval.Index] + MIS[interval.Right + 1];
                if(candidate > maxAllowedMIS)
                {
                    return false;
                }
                if (candidate > MIS[interval.Left])
                {
                    updateAt(pendingUpdates, MIS, interval.Left, candidate);
                    independentSet.setNewNextInterval(interval.Left, interval);
                }
            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> PrunedOutputSensitive::tryComputeMIS(const cg::data_structures::SharedIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> MIS(intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;
        std::vector<std::list<cg::data_structures::Interval>> indexToRelevantIntervals(intervals.end + 1);

        cg::mis::IndependentSet independentSet(intervals.size);
        // .........................
        // 5 4 3 2 1 0 
        // Increase(x)
        // 0, k-1
        // 0 0 0 0 0 0 0 0 0   [0, 0, k - 1] 
        // Increment(x)
        // [1, 0, x] [0, 0, k - 1]
        // Look-up time is O(log alpha)
        // k + alpha^2 log alpha
        for(auto right = 1; right < intervals.end + 1; ++right)
        {
            const auto& intervalsWithThisRightEndpoint = intervals.getAllIntervalsWithRightEndpoint(right - 1);
            for(auto interval : intervalsWithThisRightEndpoint) // Longest to shortest
            {
                CMIS[interval.Index] = MIS[interval.Left + 1];
                counts.Increment(Counts::IntervalOuterLoop);
                independentSet.assembleContainedIndependentSet(interval);

                const auto candidate = interval.Weight + CMIS[interval.Index];
                if (candidate > MIS[interval.Left])
                {
                    updateAt(pendingUpdates, MIS, interval.Left, candidate);
                    independentSet.setNewNextInterval(interval.Left, interval);
                }       
            }            
            auto maxCMIS = -1;
            for (auto interval : std::views::reverse(intervalsWithThisRightEndpoint)) // Shortest to longest
            {
                if(CMIS[interval.Index] > maxCMIS)
                {
                    indexToRelevantIntervals[right - 1].push_back(interval); // An interval is only added here if there is no shorter interval with a CMIS at least as large.
                    maxCMIS = CMIS[interval.Index];
                }
            }

            if (!tryUpdate(intervals, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts, indexToRelevantIntervals))
            {
                return std::nullopt;
            }
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}
