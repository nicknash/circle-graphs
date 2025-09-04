#pragma once

#include <vector>
#include <span>
#include <optional>

namespace cg::utils
{
    template<typename TCounter> class Counters;
}

namespace cg::data_structures
{
    class Interval;
    class SharedIntervalModel;
}

namespace cg::mis::shared
{
    class Naive
    {
    public:
        enum Counts
        {
            InnerLoop,
            InnerMaxLoop,
            NumMembers
        };
    private:
        static std::optional<cg::data_structures::Interval> getMaxInterval(std::span<const cg::data_structures::Interval> intervals, int maxRightEndpoint, std::vector<int> &MIS, std::vector<int> &CMIS, cg::utils::Counters<Counts>& counts);
    public:
        static std::vector<cg::data_structures::Interval> computeMIS(const cg::data_structures::SharedIntervalModel &intervals, cg::utils::Counters<Counts>& counts);
    };
}