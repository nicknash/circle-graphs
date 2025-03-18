#include <vector>
#include <stack>
#include <list>

#include "data_structures/simple_interval_rep.h"
#include "data_structures/interval.h"

#include "mis/pure_output_sensitive.h"


namespace cg::mis
{
    void PureOutputSensitive::updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newValue)
    {
        MIS[indexToUpdate] = newValue;
        pendingUpdates.push(indexToUpdate);
    }

    bool PureOutputSensitive::tryUpdate(const cg::data_structures::SimpleIntervalRep &intervals, std::stack<int> &pendingUpdates, std::vector<int>& nextRightEndpoint, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        updateAt(pendingUpdates, MIS, newInterval.Left, 1 + CMIS[newInterval.Index]);
        nextRightEndpoint[newInterval.Left] = newInterval.Right;
        while (!pendingUpdates.empty())
        {
            auto updatedIndex = pendingUpdates.top();
            pendingUpdates.pop();
            auto leftNeighbour = updatedIndex - 1;
            if (updatedIndex > 0)
            {
                if(MIS[updatedIndex] > MIS[leftNeighbour])
                {
                    nextRightEndpoint[leftNeighbour] = nextRightEndpoint[updatedIndex];
                    updateAt(pendingUpdates, MIS, leftNeighbour, MIS[updatedIndex]);
                }
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
                    nextRightEndpoint[interval.Left] = interval.Right;
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

        std::vector<int> nextRightEndpoint(intervals.end + 1, 0);
        std::vector<std::list<int>> intervalIndexToDirectlyContained(intervals.end); 

        for (auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
                auto next = nextRightEndpoint[interval.Left + 1];
                auto& containedSet = intervalIndexToDirectlyContained[interval.Index];
                while(next != 0)
                {
                    const auto& containedInterval = intervals.getIntervalByRightEndpoint(next);
                    containedSet.push_front(containedInterval.Index);
                    next = nextRightEndpoint[next];
                }
                if (!tryUpdate(intervals, pendingUpdates, nextRightEndpoint, interval, MIS, CMIS, maxAllowedMIS))
                {
                    return std::nullopt;
                }
            }
        }
        std::vector<cg::data_structures::Interval> intervalsInMis; 
        intervalsInMis.reserve(intervals.size);

        std::vector<cg::data_structures::Interval> pendingIntervals;
        pendingIntervals.reserve(intervals.size);
        auto next = nextRightEndpoint[0];
        while(next != 0)
        {
            const auto& interval = intervals.getIntervalByRightEndpoint(next);
            pendingIntervals.push_back(interval);
            while(!pendingIntervals.empty())
            {
                const auto& newInterval = pendingIntervals.back();
                pendingIntervals.pop_back();
                intervalsInMis.push_back(newInterval);
                const auto& allContained = intervalIndexToDirectlyContained[newInterval.Index];
                for(auto idx : allContained)
                {   
                    const auto& c = intervals.getIntervalByIndex(idx);
                    pendingIntervals.push_back(c);
                }
            }
            next = nextRightEndpoint[next];
        }

        return intervalsInMis;
    }
}
