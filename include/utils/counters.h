#pragma once

#include <array>
#include <cstddef>


namespace cg::utils
{
    template <typename TCounter>
    class Counters
    {
        std::array<long, static_cast<size_t>(TCounter::NumMembers)> _counts{};
    public:
        void Increment(TCounter value)
        {
            _counts[static_cast<size_t>(value)]++;
        }

        [[nodiscard]] long Get(TCounter value) const
        {
            return _counts[static_cast<size_t>(value)];
        }

        void Clear()
        {
            _counts.fill(0);
        }
    };
}