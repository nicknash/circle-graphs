#pragma once

#include <optional>
#include <span>
#include <vector>

namespace cg::data_structures
{
    class Interval;
    class SimpleIntervalRep
    {
        std::vector<Interval> intervals;
    public:
        const int End;
        const int Size;

        SimpleIntervalRep(std::span<const Interval> intervals);

        [[nodiscard]] std::optional<Interval> tryGetIntervalByRightEndpoint(int maybeRightEndpoint) const;
        [[nodiscard]] std::optional<Interval> tryGetIntervalByLeftEndpoint(int maybeLeftEndpoint) const;
    };
}