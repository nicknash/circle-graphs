#include "data_structures/simple_interval_rep.h"
#include "mis/valiente.h"
#include "mis/pure_output_sensitive.h"
#include "utils/interval_rep_utils.h"


#include "mis/switching.h"

namespace cg::mis
{
    void Switching::computeMIS(const cg::data_structures::SimpleIntervalRep &intervals)
    {
        int density = cg::utils::computeDensity(intervals);
        if (!PureOutputSensitive::tryComputeMIS(intervals, density))
        {
            Valiente::computeMIS(intervals);
        }
    }
}
