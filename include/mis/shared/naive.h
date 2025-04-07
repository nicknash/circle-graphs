#pragma once

#include <vector>
#include <span>
#include <optional>

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalRep;
}

namespace cg::mis::shared
{
    class Naive
    {
        static std::optional<cg::data_structures::Interval> getMaxInterval(std::span<const cg::data_structures::Interval> intervals, int maxRightEndpoint, std::vector<int> &MIS, std::vector<int>& CMIS);
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::SharedIntervalRep &intervals);
    };
}