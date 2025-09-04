#include <vector>
#include <stack>
#include <list>
#include <map>
#include <stdexcept>
#include <tuple>
#include <cmath>

#include <algorithm>

#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"
#include "mis/implicit_independent_set.h"
#include "mis/monotone_seq.h"
#include "utils/counters.h"

#include "mis/distinct/lazy_output_sensitive.h"


namespace cg::mis::distinct
{
    bool LazyOutputSensitive::tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals, int leftLimit, std::map<int, PendingUpdate> &pendingUpdates, ImplicitIndependentSet& independentSet, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        int maxSoFar = -1;

        while (!pendingUpdates.empty())
        {
            auto it = std::prev(pendingUpdates.end());
            auto newUpdate = it->second;
            auto currentInterval = newUpdate.interval;
            auto currentIntervalCandidate = currentInterval.Weight + CMIS[currentInterval.Index] + MIS.get(currentInterval.Right + 1);
            counts.Increment(Counts::StackOuterLoop);

            if(currentIntervalCandidate <= maxSoFar)
            {
                maxSoFar = currentIntervalCandidate;
                pendingUpdates.erase(it);
                continue;
            }
            maxSoFar = currentIntervalCandidate;
            if(currentInterval.Left < leftLimit)
            {
                break;
            }

            pendingUpdates.erase(it);

            if(currentIntervalCandidate <= MIS.get(currentInterval.Left))
            {
                continue;
            }

            cg::mis::MonotoneSeq::Range r = MIS.set(currentInterval.Left, currentIntervalCandidate);

            int nextPending = -1;
            if(!pendingUpdates.empty())
            {
                auto it2 = std::prev(pendingUpdates.end());
                nextPending = it2->first;
            }

            auto representativeMIS = currentIntervalCandidate;
            auto maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(currentInterval.Left);
            while (maybeInterval)
            {
                auto interval = maybeInterval.value();
                if(interval.Right <= nextPending)
                {
                    counts.Increment(Counts::StackInnerLoop);
                    break;
                }

                if (interval.Left < r.changeStartInclusive && interval.Right >= r.changeStartInclusive)
                {
                    counts.Increment(Counts::StackInnerLoop);
                    pendingUpdates.emplace(interval.Left, PendingUpdate{interval, -1});
                }
                maybeInterval = intervals.tryGetRightEndpointPredecessorInterval(interval.Right);
            }
        }
        return true;
    }

    std::optional<std::vector<cg::data_structures::Interval>> LazyOutputSensitive::tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts)
    {
        std::map<int, PendingUpdate> pendingUpdates;
        std::vector<int> CMIS(intervals.size);
        cg::mis::MonotoneSeq MIS(intervals.end+1);

        cg::mis::ImplicitIndependentSet independentSet(intervals.size);

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
                auto cmis = MIS.get(interval.Left + 1);
                CMIS[interval.Index] = cmis;

                independentSet.assembleContainedIndependentSet(interval);

                pendingUpdates.erase(interval.Left);
                pendingUpdates.emplace(interval.Left, PendingUpdate{interval, interval.Weight + cmis});
            }
        }
        if (!tryUpdate(intervals, 0, pendingUpdates, independentSet, MIS, CMIS, maxAllowedMIS, counts))
        {
            return std::nullopt;
        }

        const auto& intervalsInMis = independentSet.buildIndependentSet(MIS.get(0));

        return intervalsInMis;
    }
}
