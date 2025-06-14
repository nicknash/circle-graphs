#include <stdexcept>
#include <format>
#include <cstdlib>
#include <algorithm>

#include "data_structures/interval.h"

#pragma once

namespace cg::data_structures
{
    class Chord
    {
            int _first;
            int _second;
            int _index;
            int _weight;
        public:
            Chord(int first, int second, int index, int weight) : _first(first), _second(second), _index(index), _weight(weight)
            {
                if(first == second)
                {
                    throw std::runtime_error(std::format("Invalid chord end-points, they are both equal to {}", first));
                }
                if(first < 0 || second < 0)
                {
                    throw std::runtime_error(std::format("Invalid chord end-points, they must both be nonnegative but were: {}, {}", first, second));
                }   
                if(index < 0)
                {
                    throw std::runtime_error(std::format("Invalid chord index, should be nonnegative but was {}", index));
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

            [[nodiscard]] int index() const
            {
                return _index;
            }

            [[nodiscard]] int weight() const
            {
                return _weight;
            }

            [[nodiscard]] Interval asInterval() const
            {
                Interval thisAsInterval(std::min(_first, _second), std::max(_first, _second), _index, _weight);
                return thisAsInterval;
            }

            [[nodiscard]] bool intersects(const Chord& other) const
            {
                auto thisAsInterval = asInterval();
                auto otherAsInterval = other.asInterval();
                auto overlaps = thisAsInterval.overlaps(otherAsInterval);
                return overlaps;
            }

            [[nodiscard]] int length (int numEndpoints) const
            {
                auto smaller = std::min(_first, _second);
                auto larger = std::max(_first, _second);
                auto length = std::min(larger - smaller, numEndpoints + smaller - larger); // The length is defined to be the minor arc (shorter way) around the circle
                return length;
            }
    };
}