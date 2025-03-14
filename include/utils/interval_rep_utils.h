#pragma once

#include <span>


namespace cg::data_structures
{
    class Interval;
}

namespace cg::utils
{
    void verify_dense(std::span<const cg::data_structures::Interval> intervals);
}
