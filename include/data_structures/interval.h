#pragma once

#include <format>
#include <stdexcept>

namespace cg::data_structures
{
    class Interval
    {
    public:
        int Left;
        int Right;
        int Index;
        int Weight;
        Interval(int left, int right, int index, int weight);
        [[nodiscard]] bool overlaps(const Interval& other) const;
        [[nodiscard]] bool intersects(const Interval& other) const;
        [[nodiscard]] bool contains(const Interval& other) const;
        [[nodiscard]] int length() const;
    };
}

template <>
struct std::formatter<cg::data_structures::Interval> : std::formatter<std::string>
{
    auto format(const cg::data_structures::Interval &i, std::format_context &ctx) const
    {
        return std::format_to(ctx.out(), "Interval[{}]({}, {})={}", i.Index, i.Left, i.Right, i.Weight);
    }
};

namespace cg::data_structures
{
    struct IntervalDistinctLeftCompare
    {
        bool operator()(cg::data_structures::Interval const &a, cg::data_structures::Interval const &b) const
        {
            if (a.Left == b.Left)
            {
                throw new std::runtime_error(std::format("Expect to be called for intervals with distinct end-points, but was asked to compare {} and {}", a, b));
            }
            return a.Left < b.Left;
        }
    };

     struct IntervalDistinctRightCompare
    {
        bool operator()(cg::data_structures::Interval const &a, cg::data_structures::Interval const &b) const
        {
            if (a.Right == b.Right)
            {
                throw new std::runtime_error(std::format("Expect to be called for intervals with distinct end-points, but was asked to compare {} and {}", a, b));
            }
            return a.Right < b.Right;
        }
    };
}

