#include <vector>
#include <ranges>
#include <array>
#include <limits>
#include <queue>
#include <stack>
#include <algorithm>
#include <optional>

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
        Node* _initialLevelHead;
        Node* _initialLevelPrev;
        Node* _initialLevelNext;

        std::optional<InitialLevel&> _maybeInitialLevel;
        bool _mark;
    public:
        Node(int vertexId, int levelId, Node* parent);

        Node* parent() const;

        const std::vector<Node*> copy_children() const;

        void mark();

        void unmark();

        bool isMarked();

        void setLevelId(int levelId);

        int levelId();

        void addToInitialLevel(InitialLevel& initialLevel);

        const std::optional<InitialLevel&> getInitialLevel() const;

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
    InitialLevel& _initialLevel;
public:
    Forest(InitialLevel& initialLevel, std::vector<Node*>& initialLevelNodes);

    InitialLevel& initialLevel() const;

};

    // ---------------------------------------------------------------------
    // Implementation details
    // ---------------------------------------------------------------------

    // Node ----------------------------------------------------------------
    Node::Node(int vertexId, int levelId, Node* parent) : _vertexId(vertexId), _levelId(levelId), _parent(parent)
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

    void Node::addToInitialLevel(InitialLevel& initialLevel)
    {
        if(parent)
        {
            throw std::runtime_error("Cannot set initial level, this node has a parent.");
        }
        _maybeInitialLevel = initialLevel;
    }

    const std::optional<InitialLevel&> Node::getInitialLevel() const
    {
        return _maybeInitialLevel;
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
        }
        else
        {
            if(!_maybeInitialLevel)
            {
                throw std::runtime_error("This node doesn't have a parent, so its initial level should be set, but isn't.");
            }
            auto& initialLevel = _maybeInitialLevel.value();
            initialLevel.remove(this);
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
    Forest::Forest(InitialLevel& initialLevel, std::vector<Node*>& initialLevelNodes) : _initialLevel(initialLevel)
    {
        for(auto n : initialLevelNodes)
        {
            n->addToInitialLevel(initialLevel);
        }
    }

    InitialLevel& Forest::initialLevel() const
    {
        return _initialLevel;
    }

    // return all neighbours of 'a' except 'b' and except those also in neighbours(b)
    // i.e., N(a) - N(b) - {b}
    cg::data_structures::Graph::Neighbours neighbours_except(const cg::data_structures::Graph &g,
                                                             const cg::data_structures::Graph::Vertex &a,
                                                             const cg::data_structures::Graph::Vertex &b)
    {
        const auto &Na = g.neighbours(a);
        const auto &Nb = g.neighbours(b);

        cg::data_structures::Graph::Neighbours result;
        result.reserve(Na.size());
        for (auto v : Na)
        {
            // skip b, and skip any v that appears in Nb
            if (v != b && std::find(Nb.begin(), Nb.end(), v) == Nb.end())
            {
                result.push_back(v);
            }
        }
        return result;
    }

    cg::data_structures::Graph::Neighbours neighbours_intersection(const cg::data_structures::Graph::Neighbours &Na,
                                                                   const cg::data_structures::Graph::Neighbours &Nb)
    {
        cg::data_structures::Graph::Neighbours result;
        result.reserve(std::min(Na.size(), Nb.size()));
        for (auto v : Na)
        {
            if (std::find(Nb.begin(), Nb.end(), v) != Nb.end())
            {
                result.push_back(v);
            }
        }
        return result;
    }

    auto createForests(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
    {
        auto nextLevelId = 0; // This counter is globally unique, across all levels in all forests.

        auto aForestRoots = neighbours_except(g, a, b);
        auto bForestRoots = neighbours_except(g, b, a);
        auto intersectForestRoots = neighbours_intersection(g.neighbours(a), g.neighbours(b));

        std::array<cg::data_structures::Graph::Neighbours, 3> rootGroups{{
            cg::data_structures::Graph::Neighbours{aForestRoots.begin(), aForestRoots.end()},
            cg::data_structures::Graph::Neighbours{bForestRoots.begin(), bForestRoots.end()},
            intersectForestRoots 
        }};


        std::vector<Forest> forests;
        std::vector<bool> isMarked(g.numVertices());
        std::vector<Node*> vertexToNode(g.numVertices());

        isMarked[a] = true;
        isMarked[b] = true;
        for(const auto& roots : rootGroups)
        {
            for (const auto& r : roots)
            {
                isMarked[r] = true;
            }
        }

        for(const auto& roots : rootGroups)
        {
            auto forestId = forests.size();

            std::vector<Node*> currentLevel;
            currentLevel.reserve(roots.size());
            for (const auto& r : roots)
            {
                auto v = new Node(r, nextLevelId, nullptr);
                vertexToNode[r] = v;
                currentLevel.emplace_back(v);
            }
            InitialLevel initialLevel(nextLevelId);
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
                        auto child = new Node(w, nextLevelId, v);
                        vertexToNode[w] = child;
                        isMarked[w] = true;

                        v->addChild(child);
                        nextLevel.push_back(child);
                    }
                }
                ++nextLevelId;
                currentLevel = std::move(nextLevel);
            }
        }    
        return std::tuple{forests,vertexToNode, nextLevelId};
    }

    void addIfEligible(std::vector<int>& vertexIdToLastInitialLevelSize, std::queue<Node*> eligibleNodes, Node* n)
    {
        auto maybeInitialLevel = n->getInitialLevel();
        if (maybeInitialLevel)
        {
            auto initialLevelSize = maybeInitialLevel.value().size();
            auto vertexId = n->vertexId();
            auto previousSize = vertexIdToLastInitialLevelSize[vertexId];
            if (initialLevelSize <= previousSize / 2)
            {
                eligibleNodes.push(n);
                vertexIdToLastInitialLevelSize[vertexId] = initialLevelSize;
            }
        }
    }

    void markCrossEdgeTargets(const cg::data_structures::Graph &g, std::vector<Node*>& vertexToNode, Node *source, std::vector<Node*> crossEdgeTargets)
    {
        auto xVertex = source->vertexId();
        auto xNeighbours = g.neighbours(xVertex);

        auto& sourceInitialLevel = source->getInitialLevel().value(); // This must exist, because 'source' must always be part of an initial level. 

        for (auto y : xNeighbours)
        {
            auto yNode = vertexToNode[y];
        
            // Perhaps there is a more efficient way to determine if other nodes are in the same forest as 'source'
            // E.g. Perhaps by maintaining a list of edges per node somehow, and updating it as we move the node to a new forest.
            // Spinrad claims this can be done in O(degree(x)) for a vertex x, but doesn't explain how.
            auto sameForestAsSource = false;
            auto p = yNode;
            while(!p->getInitialLevel())
            {
                auto& initialLevel = p->getInitialLevel().value();
                if(initialLevel.levelId() == sourceInitialLevel.levelId())
                {
                    sameForestAsSource = true;
                    break;
                }
                p = p->parent();
            }
            if(!sameForestAsSource)
            {
                yNode->mark();
            }
        }

        // Any remaining marked nodes are a neighbour of 'source', but aren't in its forest, and so are targets of cross edges from 'source'
        for(auto y : xNeighbours)
        {
            auto yNode = vertexToNode[y];
            if(yNode->isMarked())
            {
                yNode->unmark();
                crossEdgeTargets.push_back(yNode);
            }    
        }
    }

    bool SpinradPrime::areSameSideSplit(const cg::data_structures::Graph& g, const cg::data_structures::Graph::Vertex& a, const cg::data_structures::Graph::Vertex& b)
    {
        auto [allForests, vertexToNode, nextLevelId] = createForests(g, a, b);

        std::vector<std::vector<Node*>> levelIdToExtractedNodes(2 * g.numVertices()); 
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
                seenLevelIds.push_back(oldLevelId);
                yNode->deleteFromForest();
                levelIdToExtractedNodes[oldLevelId].push_back(yNode);

            }
            crossEdgeTargets.clear();

            // Create the new forests
            for(auto levelId : seenLevelIds)
            {
                auto& newInitialLevelNodes = levelIdToExtractedNodes[levelId]; 
                for(auto n : newInitialLevelNodes)
                {
                    n->setLevelId(nextLevelId);
                }
                InitialLevel initialLevel(nextLevelId);
                Forest newForest(initialLevel, newInitialLevelNodes); 
                for(auto n : initialLevel)
                {
                    addIfEligible(vertexIdToLastInitialLevelSize, eligibleNodes, n);
                }
                ++nextLevelId;
                allForests.push_back(std::move(newForest));
                newInitialLevelNodes.clear();
            }
            seenLevelIds.clear();
        }  
        return true;
    }
}