#include <vector>
#include <stack>
#include <list>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/pure_output_sensitive.h"

namespace cg::mis
{
    void PureOutputSensitive::updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue)
    {
        MIS[indexToUpdate] = newMisValue;
        pendingUpdates.push(indexToUpdate);
    }

    bool PureOutputSensitive::tryUpdate(const cg::data_structures::SimpleIntervalRep &intervals, std::stack<int> &pendingUpdates, IndependentSet& independentSet, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        updateAt(pendingUpdates, MIS, newInterval.Left, 1 + CMIS[newInterval.Index]);
        independentSet.setNewNextRightEndpoint(newInterval.Left, newInterval.Right);
        while (!pendingUpdates.empty())
        {
            auto updatedIndex = pendingUpdates.top();
            pendingUpdates.pop();
            auto leftNeighbour = updatedIndex - 1;
            if (updatedIndex > 0 && MIS[updatedIndex] > MIS[leftNeighbour])
            {
                updateAt(pendingUpdates, MIS, leftNeighbour, MIS[updatedIndex]);
                independentSet.setSameNextRightEndpoint(leftNeighbour);
            }
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(leftNeighbour);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto candidate = 1 + CMIS[interval.Index] + MIS[interval.Right + 1];
                if (candidate > maxAllowedMIS)
                {
                    return false;
                }
                if (candidate > MIS[interval.Left])
                {
                    updateAt(pendingUpdates, MIS, interval.Left, candidate);
                    independentSet.setNewNextRightEndpoint(interval.Left, interval.Right);
                }
            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> PureOutputSensitive::tryComputeMIS(const cg::data_structures::SimpleIntervalRep &intervals, int maxAllowedMIS)
    {
        std::vector<int> MIS(intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;

        IndependentSet independentSet(intervals);

        for (auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
                independentSet.assembleContainedIndependentSet(interval);
                if (!tryUpdate(intervals, pendingUpdates, independentSet, interval, MIS, CMIS, maxAllowedMIS))
                {
                    return std::nullopt;
                }
            }
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet();
        return intervalsInMis;
    }
}
