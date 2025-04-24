#pragma once

#include <vector>
#include <map>
#include <optional>
#include <list>

namespace cg::data_structures
{
    class Interval;
}

namespace cg::mis
{
    class ImplicitIndependentSet
    {
        struct Range
        {
            int left;
            int right;
            cg::data_structures::Interval interval;
        };
        std::map<int, Range> _endpointToRange;
        std::vector<std::list<cg::data_structures::Interval>> _intervalIndexToDirectlyContained; 
    public:
        ImplicitIndependentSet(int maxNumIntervals);
        void setRange(int left, int right, const cg::data_structures::Interval& interval);
        void assembleContainedIndependentSet(const cg::data_structures::Interval &interval);
        std::vector<cg::data_structures::Interval> buildIndependentSet(int expectedCardinality); 
    };
}