#pragma once

#include "data_structures/graph.h"


namespace cg::utils
{
    class SpinradPrime
    {
            bool areSameSideSplit(const cg::data_structures::Graph& graph, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b);
        public:
            bool isPrime(const cg::data_structures::Graph& graph);
    };
}