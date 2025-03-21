#pragma once

#include <vector>
#include <list>

namespace cg::mis
{
    class IndependentSet
    {
        const cg::data_structures::SimpleIntervalRep& _intervals;
        std::vector<int> _intervalIndexToNextRightEndpoint;
        std::vector<std::list<int>> _intervalIndexToDirectlyContained; 
    public:
        IndependentSet(const cg::data_structures::SimpleIntervalRep& intervals);
        void setSameNextRightEndpoint(int where);
        void setNewNextRightEndpoint(int where, int rightEndpoint);
        void assembleContainedIndependentSet(const cg::data_structures::Interval &interval);
        std::vector<cg::data_structures::Interval> buildIndependentSet(); 
    };
}