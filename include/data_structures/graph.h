#pragma once

#include <vector>
#include <unordered_set>

namespace cg::data_structures
{
    class Graph
    {
    public:
        using Vertex = int;
        using Neighbours = std::unordered_set<Vertex>;

    private:
        std::vector<Neighbours> _vertexToNeighbours;

    public:
        explicit Graph(int numVertices);

        void addEdge(Vertex u, Vertex v);

        int numVertices() const;

        const Neighbours &neighbours(Vertex v) const;
    };
}
