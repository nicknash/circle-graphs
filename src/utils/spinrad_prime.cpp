#include <vector>
#include <ranges>
#include <array>


#include "data_structures/graph.h"
#include "utils/spinrad_prime.h"

namespace cg::utils
{
    class Node
    {
        int _id;
        int _childIdx;
    public:
        Node(int id, int childIdx) : _id(id), _childIdx(childIdx)
        {

        }

        [[nodiscard]] int id() const
        {
            return _id;
        }

        [[nodiscard]] int childIdx() const
        {
            return _childIdx;
        }

        void addChild(Node child)
        {

        }

        void setLevelId(int levelId) 
        {

        }

        int getLevelId() 
        {

        }
    };

    class Forest
    {
        std::vector<std::vector<Node>> _levels;
    public:
        Forest(const std::vector<std::vector<Node&>>& levels) : _levels(levels)
        {

        }

        const std::vector<std::vector<Node>>& levels() const 
        {
            return _levels;
        }

        std::vector<Node>& getLevel(int nodeId) const 
        {
            // return nodeIdToLevel <-- needs to be global across forests...!
        }

        std::optional<int> getLevelMark(Node n) const 
        {

        }

        void setLevelMark(std::optional<int> mark)  
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

    auto createForests(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
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

        int levelId = 0; // use this!

        std::vector<Forest> forests;
        for(const auto& roots : rootGroups)
        {
            std::vector<std::vector<Node>> levels;
            auto forestId = forests.size();

            std::vector<Node> currentLevel;
            currentLevel.reserve(roots.size());
            for (const auto& r : roots)
            {
                Node v(r);
                currentLevel.emplace_back(v);
                isMarked[r] = true;
            }
            // Perform an ordinary BFS from the roots, constructing the resultant forest, organised into levels.
            while(!currentLevel.empty())
            {
                std::vector<Node> nextLevel;

                for(auto v : currentLevel)
                {
                    auto neighbours = g.neighbours(v.id());
                    for(auto w : neighbours)
                    {
                        if(!isMarked[w])
                        {
                            isMarked[w] = true;
                            Node child(w);
                            nextLevel.push_back(child);
                            v.addChild(child);
                        }
                    }
                }
                levels.push_back(std::move(currentLevel));
                currentLevel = std::move(nextLevel);
            }
            Forest f(levels);
            forests.push_back(f);
        }    
        return std::make_tuple(forests, levelId);
    }

    bool SpinradPrime::areSameSideSplit(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
    {
        auto [allForests, levelId] = createForests(g, a, b);
        std::vector<std::vector<Node&>&> levelIdToLevel; 
        std::optional<Node> next = allForests[0].levels()[0][0];
        std::vector<bool> examined(g.numVertices(), false);
        
        std::vector<Node&> vertexToNode; // pass this to node constructor and set vertexToNode[id] = this

        std::vector<std::optional<int>> levelIdToNewLevelId(g.numVertices()); 

        
        while(next)
        {
            auto xNode = next.value();
            auto xVertex = xNode.id();
            auto xLevelId = xNode.getLevelId();
            examined[xVertex] = true;
            auto xNeighbours = g.neighbours(xVertex);
            std::vector<std::vector<Node>> newInitialLevels;
            int oldNextLevelId = levelId;
            
            
            //
            // Now examine n(x) in g:
            //    - if we find y in a different forest to x:
            //        split the level of y into neighbours and non-neigbhours of x
            //        the neighbours of x form the initial levels of new forest.
            //
            // We spend in total O(degree(x)) to do the above, as follows:
            for(auto y : xNeighbours)
            {
                auto yNode = vertexToNode[y];
                auto yLevelId = yNode.getLevelId();
                auto foundCrossEdge = xNode.getLevelId() != yLevelId; 
                if(foundCrossEdge) // {x, y} is an edge between two different forests
                {
                    auto& yLevel = levelIdToLevel[yLevelId];
                    // Remove yNode from yLevel 
                    if(yLevel.size() == 1)
                    {
                        // need to delete yLevel from the forest it is in.
                    }
                    else
                    {
                        yNode.getChildIdx();
                        // delete and swap, updating child idx
                    }

                    auto maybeNewLevelId = levelIdToNewLevelId[yLevelId];
                    
                    if(maybeNewLevelId)
                    {
                        // In this traversal of x's neighbours, we've already seen this level of y's forest, so just add to that level 
                        auto newLevelId = maybeNewLevelId.value();
                        auto& newLevel = levelIdToLevel[newLevelId];
                        newLevel.push_back(yNode);
                        yNode.setLevelId(newLevelId);
                    }
                    else
                    {
                        std::vector<Node&> newLevel;
                        newLevel.push_back(yNode);
                        yNode.setLevelId(levelId);

                        levelIdToLevel[levelId] = std::move(newLevel);
                        levelIdToNewLevelId[y] = levelId;
                        ++levelId;
                    }
                }
            }
            // Create the forests from the newly created initial levels.
            for(int i = oldNextLevelId; i < levelId; ++i)
            {
                auto newInitialLevel = levelIdToLevel[i];
                auto thisForestLevels = std::vector<std::vector<Node&>>{newInitialLevel};
                Forest f(thisForestLevels);
                allForests.push_back(f);
                levelIdToNewLevelId[i] = std::nullopt;
            }
            // Spinrad's condition sounds wrong:
            // "or the current size of x’s level is at most one half the size of x’s group during the last call to Splitall(x)"
            //
            // annoyingly he doesn't define 'group'
            // a possibly sensible interpretation would be "the number of nodes in x's level is at most half what it was the last time split-all(x) was called"
            // Note to self: don't confuse this with the DEGREE of x.

            // Now I need a record of all UNEXAMINED vertices in INITIAL levels
            //
            // During the last-call to SplitAll(x), x was in an initial level of some size S, the size of x's level didn't change


            // Find an eligible vertex x, i.e. a vertex x in the initial level of some forest such that
            //     - x has never had split-all called, OR
            //     - the number of nodes in x's level is at most half what it was the last time split-all(x) was called
            //     - if there's no eligible vertex, we're done

        }  

        /*
        We say that a vertex y goes on x’s outforest active list as soon as x and y are
put into different outforests. Vertex y leaves x’s outforest active list when we divide the
level L that contains y into neighbors and nonneighbors of x during a call to Outdivide.
        */

        /*
        Within the procedures Individe and Outdivide, we divide the level L of a forest
 containing a vertex y depending on relationships to a vertex x. All vertices in L are then
 removed from an "active list" for x and never return to the active list. We want to divide
 L using time proportional to the number of vertices in L, which would give an O(n )
 bound for the time used for dividing levels throughout the algorithm. It is easy to locate
 the position of y in constant time by maintaining pointers to the position of y in the set
 of inforests and the set of outforests. However, since the forest is changing, it may not
 be obvious how we can efficiently find other vertices in this level. After a forest is sub-
 divided, we traverse each subforest and link vertices at the same level in a doubly linked
 list. The traversal takes O(n) time, and the total number of times forests are subdivided
 is O(n), so maintaining links between vertices at the same level takes O(n 2) time. Using
 these lists, it is easy to see that the total time spent within the procedures Individe and
 Outdivide is O(n2).
        */

/*
In the new implementation, instead of calling a routine Divide (x, y) to deal with
a crossedge (x, y), we use a routine Splitall(x) to divide each forest which does not
contain x. Splitall(x) marks each vertex on x’s adjacency list, partitions any levels of
forests that contain a neighbor of x, and makes the vertices that are neighbors of x initial
levels of new forests. This splitting can be accomplished in O(IN(x)[) time in a careful
implementation.
Splitall is called by a routine corresponding to Instabilize. Every crossedge in an
undirected graph has at least one endpoint in an initial level, which makes it possible to
restrict our calls of Splitall (x) to vertices x which are in initial levels.

A vertex x in an
initial level can be used for a call Splitall (x) if either Splitall (x) has never been called,
or the current size of x’s level is at most one half the size of x’s group during the last call
to Splitall(x). This guarantees that Splitall(x) is not called more than log n + times.
If no vertex is eligible for a call to Splitall (x), let F be the forest with the largest initial
level. Every vertex in an initial level of a forest other than F has attempted to split F, so
V- F, F is a split of G.
*/
        return true;
    }
}