#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"

#include "utils/interval_model_utils.h"

#include "mif/gavril.h"

namespace cg::mif
{
    template <class T>
    struct array4
    {
        std::size_t n;
        std::vector<T> data;
        explicit array4(std::size_t n) : n(n), data(n * n * n * n) {}
        T &operator()(std::size_t i, std::size_t j, std::size_t k, std::size_t l)
        {
            return data[((i * n + j) * n + k) * n + l];
        }
        const T &operator()(std::size_t i, std::size_t j, std::size_t k, std::size_t l) const
        {
            return data[((i * n + j) * n + k) * n + l];
        }
    };

    class Node // think about correct definition here! read paper to check
    {
        std::vector<Node *> _children;

    public:
        int intervalId()
        {
        }
        void addChild(Node *child)
        {
            if (child == nullptr)
            {
                return;
            }
        }
        int numDescendants()
        {
        }
    };

    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    static void computeMif(std::span<const cg::data_structures::Interval> intervals)
    {
        const cg::data_structures::DistinctIntervalModel intervalModel(intervals);
        // The 'layers' are what Gavril calls A_0, ..., A_k at the start of page 5.
        const auto &layers = cg::interval_model_utils::createLayers(intervalModel);

        std::vector<std::vector<int>> endpointsAtLayer;

        std::vector<int> cumulativeEndpoints;
        for (const auto &l : layers)
        {
            for (const auto &i : l)
            {
                cumulativeEndpoints.push_back(i.Left);
                cumulativeEndpoints.push_back(i.Right);
            }
            endpointsAtLayer.push_back(cumulativeEndpoints);
        }

        // TODONICK: Implement the base cases.

        const auto &firstLayerEndpoints = endpointsAtLayer[0];
        const auto &firstLayer = layers[0];

        array4<Node*> FR(intervalModel.size);

        // The base case for FR_{w, 0}[x, y]:
        // Scan right to left

        for (int i = intervalModel.end - 1; i-- > 0;)
        {
            const auto &maybeInterval = intervalModel.tryGetIntervalByRightEndpoint(i);
            if (maybeInterval)
            {
                const auto &interval = maybeInterval.value(); // 'interval' is vertex 'w' in Gavril's notation
                int xIdx = 0;
                while (xIdx < firstLayerEndpoints.size() && firstLayerEndpoints[xIdx] < interval.Left)
                {
                    ++xIdx;
                }

                for (; xIdx < firstLayerEndpoints.size(); ++xIdx)
                {
                    int x = firstLayerEndpoints[xIdx];
                    int yIdx = 0;
                    while (yIdx < firstLayerEndpoints.size() && firstLayerEndpoints[yIdx] < interval.Right)
                    {
                        ++yIdx;
                    }
                    for (; yIdx < firstLayerEndpoints.size(); yIdx++)
                    {
                        int y = firstLayerEndpoints[yIdx];
                        Node *maxRoot = nullptr;
                        int maxForestSize = 0;
                        for (const auto &potentialRightChild : firstLayer)
                        {
                            // TODONICK: Account for 'dummy' sons?
                            auto isChild = potentialRightChild.Left < interval.Right && interval.Right < potentialRightChild.Right;
                            auto isWithinRange = x <= potentialRightChild.Left && potentialRightChild.Right <= y;
                            if (isChild && isWithinRange)
                            {
                                const auto &rightChild = potentialRightChild;
                                const auto forestHere = FR(x, y, rightChild.Index, 0);
                                if (forestHere != nullptr && forestHere->numDescendants() > maxForestSize)
                                {
                                    maxRoot = forestHere;
                                    maxForestSize = forestHere->numDescendants();
                                }
                            }
                        }
                        Node* newRoot = new Node();
                        newRoot->addChild(maxRoot); 
                        FR(x, y, interval.Index, 0) = newRoot;
                    }
                }
            }
        }

        auto layersSoFar = layers[0];
        // The 'layerIdx' is 'i' from Gavril's paper, that the induction of Theorem 5 is on,
        // and what FR_{w, i}, HR_{w, i}, etc are defined on.
        for (int layerIdx = 1; layerIdx < layers.size(); ++layerIdx)
        {
            // 'currentLayer' is V_k in Gavril's notation, and is the concatenation of this and all previous layers
            const auto &newLayer = layers[layerIdx];
            layersSoFar.insert(layersSoFar.end(), newLayer.begin(), newLayer.end()); // NOTE NICK: after concat no longer in left-to-right order by LEP !

            // Evaluate FL, FR for intervals in newLayer
            // Do the right-to-left scan for all intervals in currentLayer
            // Do the left-to-right scan
            // Calculate the MWIS representative
        }
    }
}