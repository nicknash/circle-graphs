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
        std::unordered_map<Vertex, Neighbours> _adj;

    public:
        int numVertices() const 
        {
            return _adj.size();
        }

        const Neighbours &neighbours(Vertex u) const
        {
            static const Neighbours empty;
            auto it = _adj.find(u);
            return it != _adj.end() ? it->second : empty;
        }

        Neighbours neighbours_copy(Vertex u) const
        {
            auto it = _adj.find(u);
            return it != _adj.end() ? it->second : Neighbours{};
        }
    };
}