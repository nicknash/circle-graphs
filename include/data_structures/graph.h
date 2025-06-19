#pragma once

#include <vector>

#include <unordered_map>
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
        Graph(int numVertices) : _vertexToNeighbours(numVertices)
        {
        }

        void addEdge(Vertex u, Vertex v)
        {
            if (u < 0 || u >= numVertices() || v < 0 || v >= numVertices())
            {
                throw std::out_of_range("Vertex index out of range");
            }
            _vertexToNeighbours[u].insert(v);
            _vertexToNeighbours[v].insert(u);
        }

        int numVertices() const
        {
            return static_cast<int>(_vertexToNeighbours.size());
        }

        const Neighbours &neighbours(Vertex v) const
        {
            if (v < 0 || v >= numVertices())
            {
                throw std::out_of_range("Vertex index out of range");
            }
            return _vertexToNeighbours[v];
        }
    };
}