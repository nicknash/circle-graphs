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
        Chord(int first, int second, int index, int weight);

        [[nodiscard]] int first() const;
        [[nodiscard]] int second() const;
        [[nodiscard]] int index() const;
        [[nodiscard]] int weight() const;

        [[nodiscard]] Interval asInterval() const;
        [[nodiscard]] bool intersects(const Chord& other) const;
        [[nodiscard]] int length(int numEndpoints) const;
    };
}
