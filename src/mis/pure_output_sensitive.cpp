#include <vector>
#include <stack>

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

    bool PureOutputSensitive::tryUpdate(const cg::data_structures::SimpleIntervalRep &intervals, std::stack<int> &pendingUpdates, const cg::data_structures::Interval &interval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS)
    {
        updateAt(pendingUpdates, MIS, interval.Left, 1 + CMIS[interval.Index]);
        while (!pendingUpdates.empty())
        {
            auto nextToUpdate = pendingUpdates.top();
            pendingUpdates.pop();
            auto leftNeighbour = nextToUpdate - 1;
            if (nextToUpdate > 0 && MIS[nextToUpdate] > MIS[leftNeighbour])
            {
                updateAt(pendingUpdates, MIS, leftNeighbour, MIS[nextToUpdate]);
            }
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(leftNeighbour);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto candidate = 1 + CMIS[interval.Index] + MIS[nextToUpdate];
                if (candidate > maxAllowedMIS)
                {
                    return false;
                }
                if (candidate > MIS[interval.Left])
                {
                    updateAt(pendingUpdates, MIS, interval.Left, candidate);
                }
            }
        }
        return true;
    }

    bool PureOutputSensitive::tryComputeMIS(const cg::data_structures::SimpleIntervalRep &intervals, int maxAllowedMIS)
    {
        std::vector<int> MIS(intervals.end, 0);
        std::vector<int> CMIS(intervals.size, 0);
        std::stack<int> pendingUpdates;
        for (auto i = 0; i < intervals.end; ++i)
        {
            auto maybeInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
                if (!tryUpdate(intervals, pendingUpdates, interval, MIS, CMIS, maxAllowedMIS))
                {
                    return false;
                }
            }
        }
        return true;
    }
}
