#include <vector>
#include <stack>
#include <list>
#include <cmath>
#include <map>

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"
#include "utils/counters.h"

#include "mis/distinct/pure_output_sensitive.h"

namespace cg::mis::distinct
{
    void PureOutputSensitive::updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue)
    {
        auto old = MIS[indexToUpdate];
        MIS[indexToUpdate] = newMisValue;  
        pendingUpdates.push(indexToUpdate);
    }

    bool PureOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, std::stack<int> &pendingUpdates, IndependentSet& independentSet, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::map<int, int, std::greater<int>> changed;
        changed.emplace(newInterval.Left, newInterval.Left);
                auto old = MIS[newInterval.Left];


        updateAt(pendingUpdates, MIS, newInterval.Left, newInterval.Weight + CMIS[newInterval.Index]);
        independentSet.setNewNextInterval(newInterval.Left, newInterval);
        //while (!pendingUpdates.empty())
        while (!changed.empty())
        {
            counts.Increment(Counts::StackOuterLoop);
            //auto updatedIndex = pendingUpdates.top();
            //pendingUpdates.pop();
            auto updatedIndex = changed.begin()->first;
            changed.erase(updatedIndex);

            auto leftNeighbour = updatedIndex - 1;
            if(leftNeighbour < 0)
            {
                continue;
            }
            if (MIS[updatedIndex] > MIS[leftNeighbour])
            {
                changed.emplace(leftNeighbour, leftNeighbour);
                updateAt(pendingUpdates, MIS, leftNeighbour, MIS[updatedIndex]);
                independentSet.setSameNextInterval(leftNeighbour);
            }
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(leftNeighbour);
            if (maybeInterval)
            {
                counts.Increment(Counts::StackInnerLoop);
                auto interval = maybeInterval.value();
                auto candidate = interval.Weight + CMIS[interval.Index] + MIS[interval.Right + 1];
                if (candidate > maxAllowedMIS)
                {
                    //TODO we should be checking the cardinality here, not the weight.
                    //return false;
                }
                if (candidate > MIS[interval.Left])
                {
                    changed.emplace(interval.Left, interval.Left);
                    updateAt(pendingUpdates, MIS, interval.Left, candidate);
                    independentSet.setNewNextInterval(interval.Left, interval);
                }
            }
        }

        for(auto c : changed)
        {
            (void)c;
        }
        while(!pendingUpdates.empty()) pendingUpdates.pop();
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> PureOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> MIS(intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;

        cg::mis::IndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            counts.Increment(Counts::IntervalOuterLoop);
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
                independentSet.assembleContainedIndependentSet(interval);
                if (!tryUpdate(intervals, pendingUpdates, independentSet, interval, MIS, CMIS, maxAllowedMIS, counts))
                {
                    return std::nullopt;
                }
            }
            //independentSet.tempDump(i);


        }
        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS[0]);

        return intervalsInMis;
    }
}
