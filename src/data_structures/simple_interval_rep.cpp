#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include "data_structures/simple_interval_rep.h"

namespace cg::data_structures
{
    SimpleIntervalRep::SimpleIntervalRep(std::span<const Interval> intervals)
        : End(2 * intervals.size()),
          Size(intervals.size())
    {
        cg::utils::verify_dense(intervals);
        // TODO: Fill in the actual little data structure. But implement the naive quadratic and Valiente's algorithm first.
    }

    [[nodiscard]] std::optional<Interval> SimpleIntervalRep::tryGetIntervalByRightEndpoint(int maybeRightEndpoint) const
    {
    }

    [[nodiscard]] std::optional<Interval> SimpleIntervalRep::tryGetIntervalByLeftEndpoint(int maybeLeftEndpoint) const
    {
    }
}