#pragma once

#include <format>

namespace cg::data_structures
{
    class Interval
    {
    public:
        int Left;
        int Right;
        int Index;
        Interval(int left, int right, int index);
        [[nodiscard]] int length() const;
    };
}

template <>
struct std::formatter<cg::data_structures::Interval> : std::formatter<std::string>
{
    auto format(const cg::data_structures::Interval &i, std::format_context &ctx) const
    {
        return std::format_to(ctx.out(), "Interval[{}]({}, {})", i.Index, i.Left, i.Right);
    }
};
