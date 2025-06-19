#include "data_structures/graph.h"
#include <stdexcept>

namespace cg::data_structures
{
    Graph::Graph(int numVertices) : _vertexToNeighbours(numVertices)
    {
    }

    void Graph::addEdge(Vertex u, Vertex v)
    {
        if (u < 0 || u >= numVertices() || v < 0 || v >= numVertices())
        {
            throw std::out_of_range("Vertex index out of range");
        }
        _vertexToNeighbours[u].insert(v);
        _vertexToNeighbours[v].insert(u);
    }

    int Graph::numVertices() const
    {
        return static_cast<int>(_vertexToNeighbours.size());
    }

    const Graph::Neighbours &Graph::neighbours(Vertex v) const
    {
        if (v < 0 || v >= numVertices())
        {
            throw std::out_of_range("Vertex index out of range");
        }
        return _vertexToNeighbours[v];
    }
}
