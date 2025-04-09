#pragma once

#include <stack>
#include <vector>
#include <optional>

namespace cg::mis
{
    class IndependentSet;
}

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalRep;
}

namespace cg::mis::shared
{
    // An implementation of the output sensitive algorithm from
    // "New Algorithms for Maximum Independent Sets of Circle Graphs", 2013 (unpublished manuscript)
    class PureOutputSensitive
    {
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
        static bool tryUpdate(const cg::data_structures::SharedIntervalRep &intervals, std::stack<int> &pendingUpdates, IndependentSet& independentSet, const cg::data_structures::Interval &newInterval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::SharedIntervalRep &intervals, int maxAllowedMIS);
    };
}
