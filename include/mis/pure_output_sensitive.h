#pragma once

#include <stack>
#include <vector>

namespace cg::mis
{
    class IndependentSet;

    // An implementation of the output sensitive algorithm from
    // "An output sensitive algorithm for computing a maximum independent set of a circle graph", 2010, Inf. Process. Lett. 110(16) pp630-634
    class PureOutputSensitive
    {
    private:
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
        static bool tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<int> &pendingUpdates, cg::mis::IndependentSet& independentSet, const cg::data_structures::Interval &interval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS);
    };
}
