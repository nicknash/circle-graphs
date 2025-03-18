#include "data_structures/simple_interval_rep.h"
#include "mis/valiente.h"
#include "mis/pure_output_sensitive.h"
#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"


#include "mis/switching.h"

namespace cg::mis
{
    std::vector<cg::data_structures::Interval> Switching::computeMIS(const cg::data_structures::SimpleIntervalRep &intervals)
    {
        int density = cg::utils::computeDensity(intervals);
        const auto& maybeMis = PureOutputSensitive::tryComputeMIS(intervals, density); 
        if (maybeMis)
        {
            return maybeMis.value();
        }

        abort(); // just to make sure I'm not executing this path for now. 
        //return Valiente::computeMIS(intervals);
    }
}
