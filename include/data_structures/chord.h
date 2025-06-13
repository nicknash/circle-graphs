#include <stdexcept>
#include <format>

#pragma once

namespace cg::data_structures
{
    class Chord
    {
            int _first;
            int _second;
        public:
            Chord(int first, int second) : _first(first), _second(second)
            {
                if(first == second)
                {
                    throw std::runtime_error(std::format("Invalid chord end-points, they are both equal to {}", first));
                }
                if(first < 0 || second < 0)
                {
                    throw std::runtime_error(std::format("Invalid chord end-points, they must both be nonnegative but were: {}, {}", first, second));
                }                
            }
            [[nodiscard]] int first() const
            {
                return _first;
            }

            [[nodiscard]] int second() const
            {
                return _second;
            }
    };
}