#pragma once

#include <stack>
#include <vector>
#include <optional>
#include <map>

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
    class DistinctIntervalModel;
}

namespace cg::mis::distinct
{
    class CombinedOutputSensitive
    {
    public:
        enum Counts
        {
            StackOuterLoop,
            StackInnerLoop,
            IntervalOuterLoop,
            NumMembers
        };
    private:
        static bool tryUpdate(const cg::data_structures::DistinctIntervalModel &intervals,  int leftLimit, std::map<int, std::optional<cg::data_structures::Interval>> &pendingUpdates, cg::mis::IndependentSet& independentSet, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalModel &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    };
}
