#pragma once

#include <stack>
#include <vector>
#include <optional>

#include <map>
#include <list>
#include <limits>


namespace cg::utils
{
    template<typename TCounter> class Counters;
}

namespace cg::data_structures
{
    class DistinctIntervalRep;
    class Interval;
}

namespace cg::mis
{
    class IndependentSet;
    class ImplicitIndependentSet;
    class MonotoneSeq;
    class IntervalStore;
}


namespace cg::mis::distinct
{
    class LazyOutputSensitive
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
        struct PendingUpdate
        {
            cg::data_structures::Interval interval;
            int candidate;
        };
        static bool tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, int leftLimit, std::map<int, PendingUpdate> &pendingUpdates,  cg::mis::ImplicitIndependentSet& independentSet, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS, cg::utils::Counters<Counts>& counts);
    };
}
