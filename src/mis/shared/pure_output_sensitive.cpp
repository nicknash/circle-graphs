#include <vector>
#include <stack>
#include <list>
#include <format>
#include <stdexcept>

#include "data_structures/shared_interval_model.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/shared/pure_output_sensitive.h"

#include "utils/counters.h"


namespace cg::mis::shared
{
    void PureOutputSensitive::updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue)
    {
        if(MIS[indexToUpdate] >= newMisValue)
        {
            throw std::runtime_error(std::format("MIS[{}] = {}, attempting to update it to {}, but the new value should be strictly greater.", indexToUpdate, MIS[indexToUpdate], newMisValue));
        }
        MIS[indexToUpdate] = newMisValue;
        pendingUpdates.push(indexToUpdate);
    }

    bool PureOutputSensitive::tryUpdate(const cg::data_structures::SharedIntervalModel &intervals, std::stack<int> &pendingUpdates, IndependentSet &independentSet, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        const auto candidate = newInterval.Weight + CMIS[newInterval.Index];
        if (candidate > MIS[newInterval.Left])
        {
            updateAt(pendingUpdates, MIS, newInterval.Left, candidate);
            independentSet.setNewNextInterval(newInterval.Left, newInterval);
        }
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
            const auto &relevantIntervals = intervals.getAllIntervalsWithRightEndpoint(leftNeighbour);
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

    std::optional<std::vector<cg::data_structures::Interval>> PureOutputSensitive::tryComputeMIS(const cg::data_structures::SharedIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> MIS(intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;

        cg::mis::IndependentSet independentSet(intervals.size);

        for(auto right = 1; right < intervals.end + 1; ++right)
        {
            const auto& intervalsWithThisRightEndpoint = intervals.getAllIntervalsWithRightEndpoint(right - 1);
            for(auto newInterval : intervalsWithThisRightEndpoint)
            {
                CMIS[newInterval.Index] = MIS[newInterval.Left + 1];
                counts.Increment(Counts::IntervalOuterLoop);
                independentSet.assembleContainedIndependentSet(newInterval);
                if(!tryUpdate(intervals, pendingUpdates, independentSet, newInterval, MIS, CMIS, maxAllowedMIS, counts))
                {
                    return std::nullopt;
                }
            }
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}
