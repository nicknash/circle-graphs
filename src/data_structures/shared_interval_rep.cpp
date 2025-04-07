#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include "data_structures/shared_interval_rep.h"

namespace cg::data_structures
{
    SharedIntervalRep::SharedIntervalRep(std::span<Interval> intervals) : 
    size(intervals.size()),
    end(intervals.size() * 2)
    {
        cg::utils::verifyEndpointsInRange(intervals);
        cg::utils::verifyIndicesDense(intervals);
    
        _leftEndpointToIntervals = std::vector<std::vector<Interval>>(end);
        for(const auto& interval : intervals)
        {
            _leftEndpointToIntervals[interval.Left].push_back(interval);
        }
    }


    [[nodiscard]] std::vector<Interval> SharedIntervalRep::getAllIntervalsWithLeftEndpoint(int leftEndpoint) const
    {
        return _leftEndpointToIntervals[leftEndpoint];
    }
    
    [[nodiscard]] Interval SharedIntervalRep::getIntervalByIndex(int intervalIndex) const
    {

    }

}