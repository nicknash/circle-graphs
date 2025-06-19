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
        using Neighbours = std::vector<Vertex>;
    
    private:
        std::vector<Neighbours> _vertexToNeighbours;

    public:
        Graph(int numVertices) : _vertexToNeighbours(numVertices)
        {

        }

        void addEdge(Vertex u, Vertex v)
        {
            _vertexToNeighbours[u].push_back(v);
            _vertexToNeighbours[v].push_back(u);
        }

        int numVertices() const 
        {
            return _vertexToNeighbours.size();
        }

        std::vector<std::tuple<int, int>> clean()
        {
            // efficiently remove duplicates from each _vertexToNeighbours, (e.g. bucket sort them)
            // return any duplicates found.
        }

        const Neighbours &neighbours(Vertex v) const
        {
            return _vertexToNeighbours[v];
        }
    };
}