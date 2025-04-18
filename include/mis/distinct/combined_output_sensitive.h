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
    class DistinctIntervalRep;
}

namespace cg::mis::distinct
{
    class CombinedOutputSensitive
    {
    private:
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
        static bool tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, cg::mis::IndependentSet& independentSet, const cg::data_structures::Interval &interval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS);
    };
}
