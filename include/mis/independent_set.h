#pragma once

#include <vector>
#include <map>
#include <optional>
#include <list>

namespace cg::mis
{
    class IndependentSet
    {
        std::vector<std::optional<cg::data_structures::Interval>> _endpointToInterval;
        std::vector<std::list<cg::data_structures::Interval>> _intervalIndexToDirectlyContained; 
    public:
        IndependentSet(int maxNumIntervals);
        void setSameNextInterval(int where);
        void setNewNextInterval(int where, const cg::data_structures::Interval& interval);
        void assembleContainedIndependentSet(const cg::data_structures::Interval &interval);
        std::vector<cg::data_structures::Interval> buildIndependentSet(int expectedCardinality); 

        void tempDump(int until);
    };

    class ImplicitIndependentSet
    {
        struct Range
        {
            int left;
            int right;
            cg::data_structures::Interval interval;
        };
        
        std::map<int, Range> _endpointToRange;
        std::map<int, cg::data_structures::Interval> _endpointToInterval;
        std::vector<std::list<cg::data_structures::Interval>> _intervalIndexToDirectlyContained; 
    public:
        ImplicitIndependentSet(int maxNumIntervals);
        void setRange(int left, int right, const cg::data_structures::Interval& interval);
        void assembleContainedIndependentSet(const cg::data_structures::Interval &interval);
        std::vector<cg::data_structures::Interval> buildIndependentSet(int expectedCardinality); 
    };
}