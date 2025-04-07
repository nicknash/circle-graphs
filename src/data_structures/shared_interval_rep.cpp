#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include "data_structures/shared_interval_rep.h"

namespace cg::data_structures
{
    SharedIntervalRep::SharedIntervalRep(std::span<const Interval> intervals)
    {
        cg::utils::verifyEndpointsInRange(intervals);
        cg::utils::verifyIndicesDense(intervals);
    }


    [[nodiscard]] std::vector<Interval> SharedIntervalRep::getAllIntervalsWithLeftEndpoint(int leftEndpoint) const
    {

    }
    
    [[nodiscard]] Interval SharedIntervalRep::getIntervalByIndex(int intervalIndex) const
    {

    }

}