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
        std::list<cg::data_structures::Interval> _intervals; // This needs to be replaced with an efficient data structure.
    public:
        IntervalStore();
        void addInterval(cg::data_structures::Interval& interval);
        std::optional<cg::data_structures::Interval> tryGetRelevantInterval(int left, int right);
    };
}