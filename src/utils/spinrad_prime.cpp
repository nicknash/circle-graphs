#include <vector>
#include <ranges>
#include <array>
#include <limits>
#include <queue>
#include <stack>
#include <algorithm>
#include <optional>
#include <format>
#include <unordered_map>

#include "data_structures/graph.h"
#include "utils/spinrad_prime.h"

namespace cg::utils
{
    class InitialLevel;

    class Node
    {
        friend class InitialLevel;
        int _vertexId;
        int _childIdx;
        int _levelId;
        std::vector<Node*> _children;
        Node* _parent;
        Node* _initialLevelPrev;
        Node* _initialLevelNext;

        InitialLevel* _initialLevel;
        bool _mark;
        int _numDeletedChildren;
    public:
        Node(int vertexId, int levelId, Node* parent);

        Node* parent() const;

        const std::vector<Node*> copy_children() const;

        int numChildren();

        void mark();

        void unmark();

        bool isMarked();

        void setLevelId(int levelId);

        int levelId();

        void addToInitialLevel(InitialLevel* initialLevel);

        InitialLevel* getInitialLevel() const;

        [[nodiscard]] int vertexId() const;

        void addChild(Node* child);

        void deleteFromForest();
    };

    class InitialLevel
    {
        int _size = 0;
        Node *head = nullptr;
        int _levelId;
    public:
        explicit InitialLevel(int levelId);

        int levelId() const;

        int size() const noexcept;

        void add(Node *n) noexcept;

        void remove(Node *n) noexcept;

        struct iterator
        {
            Node *node;

            Node *operator*() const noexcept;

            iterator &operator++() noexcept;

            bool operator!=(const iterator &o) const noexcept;
        };

        iterator begin() noexcept;

        iterator end() noexcept;
    };


class Forest
{
    InitialLevel* _initialLevel;
public:
    Forest(InitialLevel* initialLevel, std::vector<Node*>& initialLevelNodes);

    InitialLevel& initialLevel();
    std::vector<int> getCorrespondingVertices();
};

    // ---------------------------------------------------------------------
    // Implementation details
    // ---------------------------------------------------------------------

    // Node ----------------------------------------------------------------
    Node::Node(int vertexId, int levelId, Node* parent) : _vertexId(vertexId), _levelId(levelId), _parent(parent), _initialLevel(nullptr), _initialLevelNext(nullptr), _initialLevelPrev(nullptr)
    {

    }

    Node* Node::parent() const
    {
        return _parent;
    }

    const std::vector<Node*> Node::copy_children() const
    {
        std::vector<Node*> alive;
        for(auto c : _children)
        {
            if(c)
            {
                alive.push_back(c);
            }
        }
        return alive;
    }

    int Node::numChildren()
    {
        return _children.size() - _numDeletedChildren;
    }


    void Node::mark()
    {
        _mark = true;
    }

    void Node::unmark()
    {
        _mark = false;
    }

    bool Node::isMarked()
    {
        return _mark;
    }

    void Node::setLevelId(int levelId)
    {
        _levelId = levelId;
    }

    int Node::levelId()
    {
        return _levelId;
    }

    void Node::addToInitialLevel(InitialLevel* initialLevel)
    {
        if(_parent != nullptr)
        {
            throw std::runtime_error("Cannot set initial level, this node has a parent.");
        }
        _initialLevel = initialLevel;
        _initialLevel->add(this);
    }

    InitialLevel* Node::getInitialLevel() const
    {
        return _initialLevel;
    }

    int Node::vertexId() const
    {
        return _vertexId;
    }

    void Node::addChild(Node* child)
    {
        child->_childIdx = _children.size();
        _children.push_back(child);
    }

    void Node::deleteFromForest()
    {
        if(_parent)
        {
            _parent->_children[_childIdx] = nullptr;
            ++_parent->_numDeletedChildren;
            _parent = nullptr;

        }
        else
        {
            if(_initialLevel == nullptr)
            {
                throw std::runtime_error("This node doesn't have a parent, so its initial level should be set, but isn't.");
            }
            _initialLevel->remove(this);
        }
    }

    // InitialLevel ---------------------------------------------------------
    InitialLevel::InitialLevel(int levelId) : _levelId(levelId)
    {

    }

    int InitialLevel::levelId() const
    {
        return _levelId;
    }

    int InitialLevel::size() const noexcept
    {
        return _size;
    }

    void InitialLevel::add(Node *n) noexcept
    {
        n->_initialLevelPrev = nullptr;
        n->_initialLevelNext = head;

        if (head)
        {
            head->_initialLevelPrev = n;
        }

        head = n;
        ++_size;
    }

    void InitialLevel::remove(Node *n) noexcept
    {
        if (n->_initialLevelPrev)
        {
            n->_initialLevelPrev->_initialLevelNext = n->_initialLevelNext;
        }
        else
        {
            head = n->_initialLevelNext;
        }

        if (n->_initialLevelNext)
        {
            n->_initialLevelNext->_initialLevelPrev = n->_initialLevelPrev;
        }

        n->_initialLevelPrev = nullptr;
        n->_initialLevelNext = nullptr;
        --_size;
    }

    Node *InitialLevel::iterator::operator*() const noexcept
    {
        return node;
    }

    InitialLevel::iterator &InitialLevel::iterator::operator++() noexcept
    {
        node = node->_initialLevelNext;
        return *this;
    }

    bool InitialLevel::iterator::operator!=(const iterator &o) const noexcept
    {
        return node != o.node;
    }

    InitialLevel::iterator InitialLevel::begin() noexcept
    {
        return iterator{head};
    }

    InitialLevel::iterator InitialLevel::end() noexcept
    {
        return iterator{nullptr};
    }

    // Forest ---------------------------------------------------------------
    Forest::Forest(InitialLevel* initialLevel, std::vector<Node*>& initialLevelNodes) : _initialLevel(initialLevel)
    {
        for(auto n : initialLevelNodes)
        {
            n->addToInitialLevel(_initialLevel);
        }
    }

    InitialLevel& Forest::initialLevel()
    {
        return *_initialLevel;
    }

    std::vector<int> Forest::getCorrespondingVertices()
    {
        std::queue<Node*> pending;
        for(auto n : *_initialLevel)
        {
            pending.push(n);
        }
        std::vector<Node*> allNodes;

        while(!pending.empty())
        {
            auto n = pending.front();
            allNodes.push_back(n);
            pending.pop();
            for(auto child : n->copy_children())
            {
                if(!child->isMarked())
                {
                    pending.push(child);
                    child->mark();
                }
            }
        }

        std::vector<int> result;
        for(auto n : allNodes)
        {
            n->unmark();
            result.push_back(n->vertexId());
        }
        return result;
    }


    // return all neighbours of 'a' except 'b' and except those also in neighbours(b)
    // i.e., N(a) - N(b) - {b}
    std::vector<cg::data_structures::Graph::Vertex> neighbours_except(
        const cg::data_structures::Graph &g,
        const cg::data_structures::Graph::Vertex &a,
        const cg::data_structures::Graph::Vertex &b)
    {
        const auto &Na = g.neighbours(a);
        const auto &Nb = g.neighbours(b);

        std::vector<cg::data_structures::Graph::Vertex> result;
        result.reserve(Na.size());

        for (auto v : Na)
        {
            if (v != b && Nb.find(v) == Nb.end())
            {
                result.push_back(v);
            }
        }

        return result;
    }

    std::vector<cg::data_structures::Graph::Vertex> neighbours_intersection(
        const cg::data_structures::Graph::Neighbours &Na,
        const cg::data_structures::Graph::Neighbours &Nb)
    {
        std::vector<cg::data_structures::Graph::Vertex> result;
        result.reserve(std::min(Na.size(), Nb.size()));

        for (auto v : Na)
        {
            if (Nb.find(v) != Nb.end())
            {
                result.push_back(v);
            }
        }

        return result;
    }

    std::tuple<std::vector<Forest>, std::vector<Node*>, int> createForests(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
    {
        auto nextLevelId = 0; // This counter is globally unique, across all levels in all forests.

        auto aForestRoots = neighbours_except(g, a, b);
        auto bForestRoots = neighbours_except(g, b, a);
        auto intersectForestRoots = neighbours_intersection(g.neighbours(a), g.neighbours(b));

        std::array<std::vector<int>, 5> rootGroups{{
            aForestRoots,
            bForestRoots,
            intersectForestRoots,
            {a},
            {b} 
        }};


        std::vector<Forest> forests;
        std::vector<bool> isMarked(g.numVertices(), false);
        std::vector<Node*> vertexToNode(g.numVertices(), nullptr);

        for(const auto& roots : rootGroups)
        {
            for (const auto& r : roots)
            {
                isMarked[r] = true;
            }
        }

        for(const auto& roots : rootGroups)
        {
            if(roots.size() == 0)
            {
                continue;
            }
            auto forestId = forests.size();

            std::vector<Node*> currentLevel;
            currentLevel.reserve(roots.size());
            for (const auto& r : roots)
            {
                auto v = new Node(r, nextLevelId, nullptr);
                vertexToNode[r] = v;
                currentLevel.emplace_back(v);
            }
            
            auto initialLevel = new InitialLevel(nextLevelId);
            Forest f(initialLevel, currentLevel);
            ++nextLevelId;
            forests.push_back(f); 

            // Perform an ordinary BFS from the roots, constructing the resultant forest.
            while(!currentLevel.empty())
            {
                std::vector<Node*> nextLevel;
                for(auto v : currentLevel)
                {
                    auto neighbours = g.neighbours(v->vertexId());
                    for(auto w : neighbours)
                    {
                        if(!isMarked[w])
                        {
                            auto child = new Node(w, nextLevelId, v);
                            vertexToNode[w] = child;
                            isMarked[w] = true;
                            v->addChild(child);
                            nextLevel.push_back(child);
                        }
                    }
                }
                ++nextLevelId;
                currentLevel = std::move(nextLevel);
            }
        }    
        return std::tuple{forests,vertexToNode, nextLevelId};
    }

    void addIfEligible(std::vector<int>& vertexIdToLastInitialLevelSize, std::queue<Node*>& eligibleNodes, Node* n, int initialLevelSize)
    {
        auto vertexId = n->vertexId();
        auto previousSize = vertexIdToLastInitialLevelSize[vertexId];
        if (initialLevelSize <= previousSize / 2)
        {
            eligibleNodes.push(n);
            vertexIdToLastInitialLevelSize[vertexId] = initialLevelSize;
        }
    }

    void markCrossEdgeTargets(const cg::data_structures::Graph &g, std::vector<Node*>& vertexToNode, Node *source, std::vector<Node*>& crossEdgeTargets)
    {
        auto xVertex = source->vertexId();
        auto xNeighbours = g.neighbours(xVertex);
        for (auto y : xNeighbours)
        {
            auto yNode = vertexToNode[y];
            // Perhaps there is a constant time way to determine if other nodes are in the same forest as 'source'
            // E.g. Perhaps by maintaining a list of edges per node somehow, and updating it as we move the node to a new forest.
            // Spinrad claims this entire step can be done in O(degree(x)) for a vertex x (which would imply a constant time test for whether two nodes are in the same forest), but doesn't explain how.
            // It seems the problem to be solved is equivalent to split-find (so could be done in O(inverse-ackerman(n))), but 
            // perhaps I'm missing something.
            // Since I don't mind the exact time complexity for now, I'll leave this (perhaps, if I get interested in much larger graphs I'll revisit it)
            auto n = yNode;
            while (n->parent() != nullptr)
            {
                n = n->parent();
            }
            auto sourceInitialLevel = source->getInitialLevel(); // This must exist, because 'source' must always be part of an initial level.
            auto sameForestAsSource = sourceInitialLevel->levelId() == n->levelId();
            if (!sameForestAsSource)
            {
                crossEdgeTargets.push_back(yNode);
            }
        }
    }

    std::vector<Forest> SpinradPrime::getDividedForests(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
    {
        auto [allForests, vertexToNode, nextLevelId] = createForests(g, a, b);

        std::vector<std::vector<Node*>> levelIdToExtractedNodes(g.numVertices() * 2);
        std::vector<int> seenLevelIds;

        std::vector<int> vertexIdToLastInitialLevelSize(g.numVertices(), std::numeric_limits<int>::max());        
        std::queue<Node*> eligibleNodes; 
        for(auto& f : allForests)
        {
            for(auto n : f.initialLevel()) 
            {
                eligibleNodes.push(n);
            }
        }
        std::vector<Node*> crossEdgeTargets;
        while(!eligibleNodes.empty())
        {
            auto xNode = eligibleNodes.front();
            eligibleNodes.pop();
            markCrossEdgeTargets(g, vertexToNode, xNode, crossEdgeTargets);
            for (auto yNode : crossEdgeTargets)
            {
                // We're grouping nodes by the level they come from here using the globally unique levelIds, so that we can create new initial levels afterward,
                auto oldLevelId = yNode->levelId();
                auto& thisLevelNodes = levelIdToExtractedNodes[oldLevelId];
                if(thisLevelNodes.empty())
                {
                    seenLevelIds.push_back(oldLevelId);
                }
                thisLevelNodes.push_back(yNode);
            }
            crossEdgeTargets.clear();

            // Create the new forests
            for(auto levelId : seenLevelIds)
            {                
                auto& newInitialLevelNodes = levelIdToExtractedNodes[levelId];
                auto firstNode = newInitialLevelNodes[0];
                auto oldInitialLevel = firstNode->getInitialLevel();
                if (oldInitialLevel == nullptr || newInitialLevelNodes.size() < oldInitialLevel->size())
                {
                    for (auto n : newInitialLevelNodes)
                    {
                        n->deleteFromForest();
                        n->setLevelId(nextLevelId);
                    }
                    auto newInitialLevel = new InitialLevel(nextLevelId);
                    Forest newForest(newInitialLevel, newInitialLevelNodes);
                    for (auto n : *newInitialLevel)
                    {
                        addIfEligible(vertexIdToLastInitialLevelSize, eligibleNodes, n, newInitialLevel->size());
                    }
                    if (oldInitialLevel != nullptr)
                    {
                        for (auto n : *oldInitialLevel)
                        {
                            addIfEligible(vertexIdToLastInitialLevelSize, eligibleNodes, n, oldInitialLevel->size());
                        }
                    }
                    ++nextLevelId;
                    allForests.push_back(std::move(newForest));
                }
                newInitialLevelNodes.clear();
            }
            seenLevelIds.clear();
        }  
        return allForests;
    }

    std::optional<std::tuple<std::vector<int>,std::vector<int>>> SpinradPrime::trySplit(const cg::data_structures::Graph& g)
    {
        if(g.numVertices() < 4)
        {
            return std::nullopt;
        }
        int a = 0;
        int b = 1;
        int c = 2;
        // If g has a split, then some pair of any three vertices must appear on the same side of the split
        std::array<std::tuple<int, int>, 3> vertexPairs = { std::tuple{a, b}, std::tuple{a, c}, std::tuple{b, c}};

        for(auto [x, y] : vertexPairs)
        {
            auto allForests = getDividedForests(g, x, y);

            std::vector<std::vector<int>> forestVertices;

            int maxForestIndex = 0;
            int maxForestSize = 0;
            for (int i = 0; i < allForests.size(); ++i)
            {
                auto f = allForests[i];
                auto vertices = f.getCorrespondingVertices();
                forestVertices.push_back(vertices);

                if (vertices.size() > maxForestSize)
                {
                    maxForestIndex = i;
                    maxForestSize = vertices.size();
                }
            }

            if (maxForestSize > 1)
            {
                auto v2 = forestVertices[maxForestIndex];
                std::vector<int> v1;
                for (int i = 0; i < allForests.size(); ++i)
                {
                    auto v = forestVertices[i];
                    if (i != maxForestIndex)
                    {
                        v1.insert(v1.end(), v.begin(), v.end());
                    }
                }
                return std::tuple{v1, v2};
            }
        }
        return std::nullopt;
    }

    void SpinradPrime::verifySplit(const cg::data_structures::Graph& g, std::vector<int>& v1, std::vector<int>& v2)
    {
        if (v1.size() < 2)
        {
            throw std::runtime_error("v1 has less than 2 vertices!");
        }
        if (v2.size() < 2)
        {
            throw std::runtime_error("v2 has less than 2 vertices!");
        }
        std::unordered_set<int> sV1(v1.begin(), v1.end());
        std::unordered_set<int> sV2(v2.begin(), v2.end());
        std::unordered_set<int> prevNeighboursInV2;
        int prevVertex;
        for (auto x : v1)
        {
            std::unordered_set<int> neighboursInV2;

            for (auto y : g.neighbours(x))
            {
                if (sV2.contains(y))
                {
                    neighboursInV2.insert(y);
                }
            }
            if (!neighboursInV2.empty())
            {
                if (!prevNeighboursInV2.empty())
                {
                    for (auto v : neighboursInV2)
                    {
                        if (!prevNeighboursInV2.contains(v))
                        {
                            throw std::runtime_error(std::format("Not a split!: {} has neighbour {} in V2 but {} does not!", x, v, prevVertex));
                        }
                    }
                    for (auto v : prevNeighboursInV2)
                    {
                        if (!neighboursInV2.contains(v))
                        {
                            throw std::runtime_error(std::format("Not a split!: {} has neighbour {} in V2 but {} does not!", prevVertex, v, x));
                        }
                    }
                }
                prevVertex = x;
                prevNeighboursInV2 = std::move(neighboursInV2);
            }
        }
    }
}