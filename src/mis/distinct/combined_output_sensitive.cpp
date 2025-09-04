#include <vector>
#include <stack>
#include <cmath>

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "mis/independent_set.h"

#include "mis/distinct/combined_output_sensitive.h"

#include "utils/counters.h"

namespace cg::mis::distinct
{

    bool CombinedOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, int leftLimit, std::map<int, std::optional<cg::data_structures::Interval>> &pendingUpdates, IndependentSet& independentSet, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        while (!pendingUpdates.empty())
        {
            counts.Increment(Counts::StackOuterLoop);            
            auto it = std::prev(pendingUpdates.end());
            auto updatedIndex = it->first;
            if(updatedIndex < leftLimit)
            {
                break;
            }
            int newMisValue;
            auto maybeNewInterval = it->second;
            if(maybeNewInterval)
            {
                auto interval = maybeNewInterval.value();
                newMisValue = interval.Weight + CMIS[interval.Index] + MIS[interval.Right + 1];
            } 
            else
            {
                newMisValue = MIS[updatedIndex + 1];
            }
            
            pendingUpdates.erase(it);
            MIS[updatedIndex] = newMisValue;
            auto leftNeighbour = updatedIndex - 1;

            if (leftNeighbour >= 0)
            {
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
                        pendingUpdates.erase(interval.Left);
                        pendingUpdates.emplace(interval.Left, interval);
                        independentSet.setNewNextInterval(interval.Left, interval);
                    }
                }
                if (MIS[updatedIndex] > MIS[leftNeighbour])
                {
                    auto it = pendingUpdates.find(leftNeighbour);
                    if(it == pendingUpdates.end() || !it->second)
                    {
                        pendingUpdates.emplace(leftNeighbour, std::nullopt);
                        independentSet.setSameNextInterval(leftNeighbour);
                    }
                    else
                    {
                        auto existing = it->second.value();
                        auto existingMisValue = existing.Weight + CMIS[existing.Index] + MIS[existing.Right + 1];
                        if(MIS[updatedIndex] > existingMisValue)
                        {
                            pendingUpdates.erase(leftNeighbour);
                            pendingUpdates.emplace(leftNeighbour, std::nullopt);
                        }

                    }
                  
                }
            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> CombinedOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::vector<int> MIS(intervals.end + 1, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::map<int, std::optional<cg::data_structures::Interval>> pendingUpdates;

        cg::mis::IndependentSet independentSet(intervals.size);

        for (auto i = 0; i < intervals.end; ++i)
        {
            counts.Increment(Counts::IntervalOuterLoop);
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                if (!tryUpdate(intervals, interval.Left + 1, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts)) 
                { 
                    return std::nullopt;
                }
                CMIS[interval.Index] = MIS[interval.Left + 1];
                independentSet.assembleContainedIndependentSet(interval);
                pendingUpdates.erase(interval.Left);
                pendingUpdates.emplace(interval.Left, interval);
            }
        }
        if (!tryUpdate(intervals, 0, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts))
        {
            return std::nullopt;
        }
        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS[0]);
        return intervalsInMis;
    }
}
