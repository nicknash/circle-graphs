#pragma once

#include <stack>
#include <vector>
#include <optional>

#include <map>
#include <limits>

namespace cg::mis
{
    class IndependentSet;
    class ImplicitIndependentSet;
    class MonotoneSeq;
}

namespace cg::data_structures
{
    class DistinctIntervalRep;
    class Interval;
}

namespace cg::mis::distinct
{
    class ImplicitOutputSensitive
    {
    private:
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
        static bool tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, cg::mis::ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &interval, cg::mis::MonotoneSeq &MIS, std::vector<int> &CMIS, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS);
    };
}
