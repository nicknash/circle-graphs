#pragma once

#include "data_structures/graph.h"
#include <vector>
#include <tuple>

namespace cg::utils
{
    class Forest;

    class SpinradPrime
    {
            std::vector<Forest> getDividedForests(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b);
        public:
            std::optional<std::tuple<std::vector<int>,std::vector<int>>> trySplit(const cg::data_structures::Graph& graph);
    };
}