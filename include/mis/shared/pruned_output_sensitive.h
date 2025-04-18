#pragma once

#include <stack>
#include <vector>
#include <optional>
#include <list>

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
    class SharedIntervalRep;
}

namespace cg::mis::shared
{
    class PrunedOutputSensitive
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
        static bool tryUpdate(const cg::data_structures::SharedIntervalRep &intervals, std::stack<int> &pendingUpdates, IndependentSet& independentSet, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts, std::vector<std::list<cg::data_structures::Interval>>& indexToRelevantIntervals);
    public:
        
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::SharedIntervalRep &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    };
}
