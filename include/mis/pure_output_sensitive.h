#pragma once

#include <stack>
#include <vector>

class SimpleIntervalRep;
class Interval;

namespace cg::mis
{
    // An implementation of the output sensitive algorithm from
    // "An output sensitive algorithm for computing a maximum independent set of a circle graph", 2010, Inf. Process. Lett. 110(16) pp630-634
    class PureOutputSensitive
    {
    private:
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newValue);
        static bool tryUpdate(const cg::data_structures::SimpleIntervalRep &intervals, std::stack<int> &pendingUpdates, const cg::data_structures::Interval &interval, std::vector<int> &MIS, std::vector<int> &CMIS, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::SimpleIntervalRep &intervals, int maxAllowedMIS);
    };
}
