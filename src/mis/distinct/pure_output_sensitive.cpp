#include <vector>
#include <stack>
#include <list>

#include "data_structures/distinct_interval_rep.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/distinct/pure_output_sensitive.h"

#include <iostream>

namespace cg::mis::distinct
{
    void PureOutputSensitive::updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue)
    {
        MIS[indexToUpdate] = newMisValue;  
        pendingUpdates.push(indexToUpdate);
    }

    bool PureOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, IndependentSet& independentSet, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        updateAt(pendingUpdates, MIS, newInterval.Left, newInterval.Weight + CMIS[newInterval.Index]);
        independentSet.setNewNextInterval(newInterval.Left, newInterval);
        while (!pendingUpdates.empty())
        {
            auto updatedIndex = pendingUpdates.top();
            pendingUpdates.pop();
            auto leftNeighbour = updatedIndex - 1;
            if (leftNeighbour >= 0 && MIS[updatedIndex] > MIS[leftNeighbour])
            {
                updateAt(pendingUpdates, MIS, leftNeighbour, MIS[updatedIndex]);
                independentSet.setSameNextInterval(leftNeighbour);
            }
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(leftNeighbour);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto candidate = interval.Weight + CMIS[interval.Index] + MIS[interval.Right + 1];
                if (candidate > maxAllowedMIS)
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

    std::optional<std::vector<cg::data_structures::Interval>> PureOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS)
    {
        std::vector<int> MIS(intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;

        cg::mis::IndependentSet independentSet(intervals.size);

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
            // std::cout << i << " :: ";

            // for(auto x = 0; x < i; ++x)
            // {
            //     std::cout << MIS[x] << " ";
            // }
            // std::cout << std::endl;
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}
