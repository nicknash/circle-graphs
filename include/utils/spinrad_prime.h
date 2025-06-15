#pragma once

namespace cg::data_structures
{
    class Graph;
}

namespace cg::utils
{
    class SpinradPrime
    {
            bool areSameSideSplit(const cg::data_structures::Graph& intervalModel, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b);
        public:
            bool isPrime(const cg::data_structures::Graph& intervalModel);
    };
}