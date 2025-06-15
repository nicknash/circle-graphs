#include <vector>
#include <ranges>
#include <array>


#include "data_structures/graph.h"
#include "utils/spinrad_prime.h"

namespace cg::utils
{
    class Node
    {
    public:

    };

    class Forest
    {
    public:
        Forest(const std::vector<std::vector<Node>>& levels)
        {

        }
    };

    auto neighbours_except(const cg::data_structures::Graph &g, const cg::data_structures::Graph::Vertex &a, const cg::data_structures::Graph::Vertex &b)
    {
        const auto &Na = g.neighbours(a);
        const auto &Nb = g.neighbours(b);
        return Na | std::views::filter([&, b](cg::data_structures::Graph::Vertex v)
                                       { return v != b && Nb.find(v) == Nb.end(); });
    }

    cg::data_structures::Graph::Neighbours neighbours_intersection(const cg::data_structures::Graph::Neighbours &Na, const cg::data_structures::Graph::Neighbours &Nb)
    {
        cg::data_structures::Graph::Neighbours result;
        result.reserve(std::min(Na.size(), Nb.size()));
        for (auto v : Na)
        {
            if (Nb.contains(v))
            {
                result.insert(v);
            }
        }
        return result;
    }

    bool SpinradPrime::areSameSideSplit(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
    {
        std::vector<bool> isMarked(g.numVertices());

        auto aForestRoots = neighbours_except(g, a, b);
        auto bForestRoots = neighbours_except(g, b, a);
        auto intersectForestRoots = neighbours_intersection(g.neighbours(a), g.neighbours(b));

        std::array<cg::data_structures::Graph::Neighbours, 3> rootGroups{{
            cg::data_structures::Graph::Neighbours{aForestRoots.begin(), aForestRoots.end()},
            cg::data_structures::Graph::Neighbours{bForestRoots.begin(), bForestRoots.end()},
            intersectForestRoots 
        }};

        for(const auto& roots : rootGroups)
        {
            std::vector<std::vector<Node>> levels;
            
        }

        return true;
    }
}