#pragma once

#include <stack>
#include <vector>
#include <optional>

namespace cg::utils
{
    template<typename TCounter> class Counters;
}

namespace cg::mis
{
    class IndependentSet;
}

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalModel;
}

namespace cg::mis::shared
{
    // An implementation of the output sensitive algorithm from
    // "New Algorithms for Maximum Independent Sets of Circle Graphs", 2013 (unpublished manuscript)
    class PureOutputSensitive
    {
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
    public:
        enum Counts
        {
            StackOuterLoop,
            StackInnerLoop,
            IntervalOuterLoop,
            NumMembers
        };
    private:
        static bool tryUpdate(const cg::data_structures::SharedIntervalModel &intervals, std::stack<int> &pendingUpdates, IndependentSet& independentSet, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    public:
        
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::SharedIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    };
}
