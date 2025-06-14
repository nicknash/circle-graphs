#include "data_structures/chord.h"

namespace cg::data_structures
{
    Chord::Chord(int first, int second, int index, int weight) : _first(first), _second(second), _index(index), _weight(weight)
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

    [[nodiscard]] int Chord::first() const
    {
        return _first;
    }

    [[nodiscard]] int Chord::second() const
    {
        return _second;
    }

    [[nodiscard]] int Chord::index() const
    {
        return _index;
    }

    [[nodiscard]] int Chord::weight() const
    {
        return _weight;
    }

    [[nodiscard]] Interval Chord::asInterval() const
    {
        Interval thisAsInterval(std::min(_first, _second), std::max(_first, _second), _index, _weight);
        return thisAsInterval;
    }

    [[nodiscard]] bool Chord::intersects(const Chord& other) const
    {
        auto thisAsInterval = asInterval();
        auto otherAsInterval = other.asInterval();
        auto overlaps = thisAsInterval.overlaps(otherAsInterval);
        return overlaps;
    }

    [[nodiscard]] int Chord::length(int numEndpoints) const
    {
        auto smaller = std::min(_first, _second);
        auto larger = std::max(_first, _second);
        auto length = std::min(
            larger - smaller,
            numEndpoints + smaller - larger); // The length is defined to be the minor arc (shorter way) around the circle
        return length;
    }
}

