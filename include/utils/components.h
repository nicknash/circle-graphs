#pragma once

#include <vector>
#include <span>

namespace cg::data_structures
{
    class Interval;
}

namespace cg::components
{
    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponents(std::span<const cg::data_structures::Interval> intervals);
    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponentsNaive(std::span<const cg::data_structures::Interval> intervals);
}