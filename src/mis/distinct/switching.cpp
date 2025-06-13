#include "data_structures/distinct_interval_rep.h"
#include "mis/distinct/valiente.h"
#include "mis/distinct/pure_output_sensitive.h"
#include "utils/interval_rep_utils.h"
#include "utils/counters.h"
#include "data_structures/interval.h"


#include "mis/distinct/switching.h"

namespace cg::mis::distinct
{
    std::vector<cg::data_structures::Interval> Switching::computeMIS(const cg::data_structures::DistinctIntervalModel &intervals)
    {
        int density = cg::utils::computeDensity(intervals);
        cg::utils::Counters<PureOutputSensitive::Counts> counts;
        const auto& maybeMis = PureOutputSensitive::tryComputeMIS(intervals, density, counts); 
        if (maybeMis)
        {
            return maybeMis.value();
        }
        return Valiente::computeMIS(intervals);
    }
}
