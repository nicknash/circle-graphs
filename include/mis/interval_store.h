#pragma once

#include <vector>
#include <optional>
#include <list>

namespace cg::data_structures
{
    class Interval;
}

namespace cg::mis
{

    class IntervalStore
    {
        std::vector<std::list<cg::data_structures::Interval>> _cmisToIntervals; // This needs to be replaced with an efficient data structure.
        int _maxContainedMIS;
    public:
        IntervalStore(int numIntervals);
        void addInterval(int containedMIS, cg::data_structures::Interval& interval);
        std::optional<cg::data_structures::Interval> tryGetRelevantInterval(int containedMIS, int left, int right);
        int getMaxContainedMIS();
    };
}