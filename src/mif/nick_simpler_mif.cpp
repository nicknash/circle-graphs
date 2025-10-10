#include "mif/nick_simpler_mif.h"

#include "data_structures/interval.h"
#include "utils/array_utils.h"
#include <algorithm>
#include <vector>

namespace
{
    using Interval = cg::data_structures::Interval;

    void buildChildren(const std::vector<Interval> &intervals,
                       std::vector<std::vector<int>> &leftChildren,
                       std::vector<std::vector<int>> &rightChildren)
    {
        const int numIntervals = static_cast<int>(intervals.size());
        for (int intervalIndex = 0; intervalIndex < numIntervals; ++intervalIndex)
        {
            const auto &outerInterval = intervals[intervalIndex];
            for (int childIndex = 0; childIndex < numIntervals; ++childIndex)
            {
                if (childIndex == intervalIndex)
                {
                    continue;
                }
                const auto &innerInterval = intervals[childIndex];
                if (innerInterval.Left < outerInterval.Left && outerInterval.Left < innerInterval.Right &&
                    innerInterval.Right < outerInterval.Right)
                {
                    leftChildren[intervalIndex].push_back(childIndex);
                }
                if (outerInterval.Left < innerInterval.Left && innerInterval.Left < outerInterval.Right &&
                    outerInterval.Right < innerInterval.Right)
                {
                    rightChildren[intervalIndex].push_back(childIndex);
                }
            }

            const auto compareByLeft = [&intervals](int lhs, int rhs) {
                return intervals[lhs].Left < intervals[rhs].Left;
            };
            std::sort(leftChildren[intervalIndex].begin(), leftChildren[intervalIndex].end(), compareByLeft);
            std::sort(rightChildren[intervalIndex].begin(), rightChildren[intervalIndex].end(), compareByLeft);
        }
    }

    void computeLeftDummies(int offset,
                            const std::vector<Interval> &intervals,
                            int numIntervals,
                            const cg::utils::array3<int> &leftForests,
                            const cg::utils::array3<int> &rightForests,
                            cg::utils::array2<int> &leftDummies)
    {
        for (int intervalIndex = 0; intervalIndex < numIntervals; ++intervalIndex)
        {
            const auto &interval = intervals[intervalIndex];
            const int dummyPosition = interval.Left + offset;
            if (interval.Left + 1 > dummyPosition || dummyPosition > interval.Right - 1)
            {
                continue;
            }

            int best = 0;
            for (int candidate = 0; candidate < numIntervals; ++candidate)
            {
                if (candidate == intervalIndex)
                {
                    continue;
                }
                const auto &inner = intervals[candidate];
                if (inner.Left > interval.Left && inner.Right <= dummyPosition)
                {
                    for (int split = inner.Left; split < dummyPosition; ++split)
                    {
                        const int leftScore = leftForests(candidate, inner.Left, split);
                        const int rightScore = rightForests(candidate, split + 1, dummyPosition);
                        best = std::max(best, 1 + leftScore + rightScore);
                    }
                }
            }
            leftDummies(intervalIndex, dummyPosition) = best;
        }
    }

    void computeRightDummies(int offset,
                             const std::vector<Interval> &intervals,
                             int numIntervals,
                             int maxEndpoint,
                             const cg::utils::array3<int> &leftForests,
                             const cg::utils::array3<int> &rightForests,
                             cg::utils::array3<int> &rightDummies)
    {
        for (int intervalIndex = 0; intervalIndex < numIntervals; ++intervalIndex)
        {
            const auto &interval = intervals[intervalIndex];
            for (int leftBound = 0; leftBound <= maxEndpoint; ++leftBound)
            {
                const int tail = std::max(leftBound, interval.Right + 1);
                const int rightBound = tail + offset;
                if (rightBound < interval.Right || rightBound > maxEndpoint)
                {
                    continue;
                }
                if (tail > rightBound)
                {
                    rightDummies(intervalIndex, leftBound, rightBound) = 0;
                    continue;
                }

                int best = 0;
                for (int candidate = 0; candidate < numIntervals; ++candidate)
                {
                    if (candidate == intervalIndex)
                    {
                        continue;
                    }
                    const auto &inner = intervals[candidate];
                    if (inner.Left >= tail && inner.Right <= rightBound)
                    {
                        for (int split = inner.Left; split < rightBound; ++split)
                        {
                            const int leftScore = leftForests(candidate, inner.Left, split);
                            const int rightScore = rightForests(candidate, split + 1, rightBound);
                            best = std::max(best, 1 + leftScore + rightScore);
                        }
                    }
                }

                rightDummies(intervalIndex, leftBound, rightBound) = best;
            }
        }
    }

    [[nodiscard]] int computeLeftChain(int intervalIndex,
                                       int leftBound,
                                       int rightBound,
                                       const std::vector<Interval> &intervals,
                                       const std::vector<std::vector<int>> &leftChildren,
                                       const cg::utils::array3<int> &leftForests,
                                       const cg::utils::array3<int> &rightForests,
                                       const cg::utils::array2<int> &leftDummies)
    {
        const auto &interval = intervals[intervalIndex];
        const auto &children = leftChildren[intervalIndex];

        const int aMin = leftBound;
        const int aMax = interval.Left;
        const int bMin = interval.Left;
        const int bMax = rightBound;

        const int aSize = std::max(0, aMax - aMin + 1);
        const int bSize = std::max(0, bMax - bMin + 1);
        if (aSize == 0 || bSize == 0)
        {
            return 0;
        }

        cg::utils::array2<int> nextValues(aSize, bSize, 0);
        cg::utils::array2<int> currentValues(aSize, bSize, 0);

        for (int a = aMin; a <= aMax; ++a)
        {
            for (int b = bMin; b <= bMax; ++b)
            {
                int best = 0;
                const int qLower = std::max(interval.Left + 1, a);
                const int qUpper = std::min(b, interval.Right - 1);
                if (qLower <= qUpper)
                {
                    for (int q = qLower; q <= qUpper; ++q)
                    {
                        best = std::max(best, leftDummies(intervalIndex, q));
                    }
                }
                nextValues(a - aMin, b - bMin) = best;
            }
        }

        for (int childPosition = static_cast<int>(children.size()) - 1; childPosition >= 0; --childPosition)
        {
            const int childIndex = children[childPosition];
            const auto &childInterval = intervals[childIndex];

            for (int a = aMin; a <= aMax; ++a)
            {
                for (int b = bMin; b <= bMax; ++b)
                {
                    int best = nextValues(a - aMin, b - bMin);
                    if (a <= childInterval.Left && childInterval.Right <= b)
                    {
                        const int zLower = std::max(a, childInterval.Left);
                        const int zUpper = interval.Left - 1;
                        const int yLower = interval.Left + 1;
                        const int yUpper = std::min(b, childInterval.Right);
                        if (zLower <= zUpper && yLower <= yUpper)
                        {
                            for (int z = zLower; z <= zUpper; ++z)
                            {
                                for (int y = yLower; y <= yUpper; ++y)
                                {
                                    const int component = 1 + leftForests(childIndex, a, z) +
                                                          rightForests(childIndex, y, b);
                                    const int suffix = nextValues(z - aMin, y - bMin);
                                    best = std::max(best, component + suffix);
                                }
                            }
                        }
                    }
                    currentValues(a - aMin, b - bMin) = best;
                }
            }
            std::swap(nextValues.data, currentValues.data);
        }

        return nextValues(leftBound - aMin, rightBound - bMin);
    }

    [[nodiscard]] int computeRightChain(int intervalIndex,
                                        int leftBound,
                                        int rightBound,
                                        const std::vector<Interval> &intervals,
                                        const std::vector<std::vector<int>> &rightChildren,
                                        const cg::utils::array3<int> &leftForests,
                                        const cg::utils::array3<int> &rightForests,
                                        const cg::utils::array3<int> &rightDummies)
    {
        const auto &interval = intervals[intervalIndex];
        const auto &children = rightChildren[intervalIndex];

        const int aMin = leftBound;
        const int aMax = interval.Right;
        const int bMin = interval.Right;
        const int bMax = rightBound;

        const int aSize = std::max(0, aMax - aMin + 1);
        const int bSize = std::max(0, bMax - bMin + 1);
        if (aSize == 0 || bSize == 0)
        {
            return 0;
        }

        cg::utils::array2<int> nextValues(aSize, bSize, 0);
        cg::utils::array2<int> currentValues(aSize, bSize, 0);

        for (int a = aMin; a <= aMax; ++a)
        {
            for (int b = bMin; b <= bMax; ++b)
            {
                int best = 0;
                const int yLower = std::max(a, interval.Right);
                if (yLower <= b)
                {
                    for (int y = yLower; y <= b; ++y)
                    {
                        best = std::max(best, rightDummies(intervalIndex, a, y));
                    }
                }
                nextValues(a - aMin, b - bMin) = best;
            }
        }

        for (int childPosition = static_cast<int>(children.size()) - 1; childPosition >= 0; --childPosition)
        {
            const int childIndex = children[childPosition];
            const auto &childInterval = intervals[childIndex];

            for (int a = aMin; a <= aMax; ++a)
            {
                for (int b = bMin; b <= bMax; ++b)
                {
                    int best = nextValues(a - aMin, b - bMin);
                    if (a <= childInterval.Left && childInterval.Right <= b)
                    {
                        const int qLower = std::max(a, childInterval.Left);
                        const int qUpper = interval.Right - 1;
                        const int xLower = interval.Right + 1;
                        const int xUpper = std::min(b, childInterval.Right);
                        if (qLower <= qUpper && xLower <= xUpper)
                        {
                            for (int q = qLower; q <= qUpper; ++q)
                            {
                                for (int x = xLower; x <= xUpper; ++x)
                                {
                                    const int component = 1 + leftForests(childIndex, a, q) +
                                                          rightForests(childIndex, x, b);
                                    const int suffix = nextValues(q - aMin, x - bMin);
                                    best = std::max(best, component + suffix);
                                }
                            }
                        }
                    }
                    currentValues(a - aMin, b - bMin) = best;
                }
            }
            std::swap(nextValues.data, currentValues.data);
        }

        return nextValues(leftBound - aMin, rightBound - bMin);
    }

    void computeFullChains(int offset,
                           const std::vector<Interval> &intervals,
                           const std::vector<std::vector<int>> &leftChildren,
                           const std::vector<std::vector<int>> &rightChildren,
                           int numIntervals,
                           int maxEndpoint,
                           cg::utils::array3<int> &leftForests,
                           cg::utils::array3<int> &rightForests,
                           const cg::utils::array2<int> &leftDummies,
                           const cg::utils::array3<int> &rightDummies)
    {
        for (int leftBound = 0; leftBound <= maxEndpoint - offset; ++leftBound)
        {
            const int rightBound = leftBound + offset;
            for (int intervalIndex = 0; intervalIndex < numIntervals; ++intervalIndex)
            {
                const auto &interval = intervals[intervalIndex];
                if (leftBound <= interval.Left && interval.Left <= rightBound)
                {
                    leftForests(intervalIndex, leftBound, rightBound) =
                        computeLeftChain(intervalIndex, leftBound, rightBound, intervals, leftChildren, leftForests,
                                         rightForests, leftDummies);
                }
                else
                {
                    leftForests(intervalIndex, leftBound, rightBound) = 0;
                }

                if (leftBound <= interval.Right && interval.Right <= rightBound)
                {
                    rightForests(intervalIndex, leftBound, rightBound) =
                        computeRightChain(intervalIndex, leftBound, rightBound, intervals, rightChildren, leftForests,
                                          rightForests, rightDummies);
                }
                else
                {
                    rightForests(intervalIndex, leftBound, rightBound) = 0;
                }
            }
        }
    }
}

namespace cg::mif
{
    int NickSimplerMif::computeMifSize(const cg::data_structures::DistinctIntervalModel &intervalModel)
    {
        const std::vector<Interval> intervals = intervalModel.getAllIntervals();
        const int numIntervals = static_cast<int>(intervals.size());
        if (numIntervals == 0)
        {
            return 0;
        }

        const int maxEndpoint = intervalModel.end > 0 ? intervalModel.end - 1 : 0;
        const int endpointCount = std::max(1, maxEndpoint + 1);

        std::vector<std::vector<int>> leftChildren(numIntervals);
        std::vector<std::vector<int>> rightChildren(numIntervals);
        buildChildren(intervals, leftChildren, rightChildren);

        cg::utils::array3<int> leftForests(numIntervals, endpointCount, endpointCount, 0);
        cg::utils::array3<int> rightForests(numIntervals, endpointCount, endpointCount, 0);
        cg::utils::array2<int> leftDummies(numIntervals, endpointCount, 0);
        cg::utils::array3<int> rightDummies(numIntervals, endpointCount, endpointCount, 0);

        for (int offset = 0; offset <= maxEndpoint; ++offset)
        {
            computeLeftDummies(offset, intervals, numIntervals, leftForests, rightForests, leftDummies);
            computeRightDummies(offset, intervals, numIntervals, maxEndpoint, leftForests, rightForests, rightDummies);
            computeFullChains(offset, intervals, leftChildren, rightChildren, numIntervals, maxEndpoint, leftForests,
                              rightForests, leftDummies, rightDummies);
        }

        int best = 0;
        for (int intervalIndex = 0; intervalIndex < numIntervals; ++intervalIndex)
        {
            const auto &interval = intervals[intervalIndex];
            for (int split = interval.Left; split < interval.Right; ++split)
            {
                const int leftScore = leftForests(intervalIndex, 0, split);
                const int rightScore = rightForests(intervalIndex, split + 1, maxEndpoint);
                best = std::max(best, 1 + leftScore + rightScore);
            }
        }
        return best;
    }
}

