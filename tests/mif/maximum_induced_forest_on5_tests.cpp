#include "doctest/doctest.h"

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"
#include "mif/maximum_induced_forest_on5.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

namespace cgtd = cg::data_structures;

namespace
{
    [[nodiscard]] cgtd::Interval mk(int L, int R, int idx, int w = 1)
    {
        if (L > R)
        {
            std::swap(L, R);
        }
        return cgtd::Interval(L, R, idx, w);
    }

    void validateEndpoints(const std::vector<cgtd::Interval> &intervals)
    {
        const int n = static_cast<int>(intervals.size());
        REQUIRE(n > 0);
        std::vector<int> endpoints;
        endpoints.reserve(2 * n);
        for (const auto &interval : intervals)
        {
            endpoints.push_back(interval.Left);
            endpoints.push_back(interval.Right);
        }
        std::sort(endpoints.begin(), endpoints.end());
        for (int i = 0; i < 2 * n; ++i)
        {
            REQUIRE(endpoints[i] == i);
        }
    }

    bool overlapsStrict(const cgtd::Interval &a, const cgtd::Interval &b)
    {
        return (a.Left < b.Left && b.Left < a.Right && a.Right < b.Right) ||
               (b.Left < a.Left && a.Left < b.Right && b.Right < a.Right);
    }

    std::vector<std::vector<int>> buildAdjacency(const std::vector<cgtd::Interval> &intervals)
    {
        const int n = static_cast<int>(intervals.size());
        std::vector<std::vector<int>> graph(n);
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                if (overlapsStrict(intervals[i], intervals[j]))
                {
                    graph[i].push_back(j);
                    graph[j].push_back(i);
                }
            }
        }
        return graph;
    }

    bool isForestInduced(const std::vector<cgtd::Interval> &intervals, const std::vector<int> &picked)
    {
        const auto graph = buildAdjacency(intervals);
        std::unordered_map<int, int> indexOf;
        indexOf.reserve(picked.size());
        for (int pos = 0; pos < static_cast<int>(picked.size()); ++pos)
        {
            indexOf.emplace(picked[pos], pos);
        }
        const int k = static_cast<int>(picked.size());
        std::vector<std::vector<int>> subgraph(k);
        for (int pos = 0; pos < k; ++pos)
        {
            const int original = picked[pos];
            for (int neighbor : graph[original])
            {
                auto it = indexOf.find(neighbor);
                if (it != indexOf.end())
                {
                    subgraph[pos].push_back(it->second);
                }
            }
        }
        std::vector<int> color(k, 0);
        std::function<bool(int, int)> dfs = [&](int node, int parent)
        {
            color[node] = 1;
            for (int neighbor : subgraph[node])
            {
                if (neighbor == parent)
                {
                    continue;
                }
                if (color[neighbor] == 1)
                {
                    return false;
                }
                if (color[neighbor] == 0 && !dfs(neighbor, node))
                {
                    return false;
                }
            }
            color[node] = 2;
            return true;
        };
        for (int i = 0; i < k; ++i)
        {
            if (color[i] == 0 && !dfs(i, -1))
            {
                return false;
            }
        }
        return true;
    }

    int bruteForceMifSize(const std::vector<cgtd::Interval> &intervals)
    {
        const int n = static_cast<int>(intervals.size());
        REQUIRE(n <= 20);
        int best = 0;
        std::vector<int> picked;
        picked.reserve(n);
        for (unsigned mask = 1; mask < (1u << n); ++mask)
        {
            picked.clear();
            for (int i = 0; i < n; ++i)
            {
                if (mask & (1u << i))
                {
                    picked.push_back(i);
                }
            }
            if (static_cast<int>(picked.size()) <= best)
            {
                continue;
            }
            if (isForestInduced(intervals, picked))
            {
                best = static_cast<int>(picked.size());
            }
        }
        return best;
    }

    std::vector<cgtd::Interval> makeIntervalsFromPermutation(const std::vector<int> &perm)
    {
        const int n = static_cast<int>(perm.size()) / 2;
        std::vector<cgtd::Interval> intervals;
        intervals.reserve(n);
        for (int i = 0; i < n; ++i)
        {
            const int a = perm[2 * i];
            const int b = perm[2 * i + 1];
            intervals.push_back(mk(a, b, i));
        }
        validateEndpoints(intervals);
        return intervals;
    }

    int maximumInducedForestOn5Size(const std::vector<cgtd::Interval> &intervals)
    {
        cgtd::DistinctIntervalModel model(intervals);
        return cg::mif::MaximumInducedForestOn5::computeMifSize(model);
    }
}

TEST_CASE("[MaximumInducedForestOn5] Single edge is fully kept")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 2, 0));
    intervals.push_back(mk(1, 3, 1));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 2);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Single dummy left child")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 3, 0));
    intervals.push_back(mk(1, 2, 1));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Three nested intervals")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 5, 0));
    intervals.push_back(mk(1, 4, 1));
    intervals.push_back(mk(2, 3, 2));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 3);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Four nested intervals")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 7, 0));
    intervals.push_back(mk(1, 6, 1));
    intervals.push_back(mk(2, 5, 2));
    intervals.push_back(mk(3, 4, 3));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 4);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Two disjoint intervals")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 1, 0));
    intervals.push_back(mk(2, 3, 1));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 2);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Three disjoint intervals")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 1, 0));
    intervals.push_back(mk(2, 3, 1));
    intervals.push_back(mk(4, 5, 2));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 3);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Single interval containing two disjoint")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 5, 0));
    intervals.push_back(mk(1, 2, 1));
    intervals.push_back(mk(3, 4, 2));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 3);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Single interval containing three disjoint")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 7, 0));
    intervals.push_back(mk(1, 2, 1));
    intervals.push_back(mk(3, 4, 2));
    intervals.push_back(mk(5, 6, 3));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 4);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Triangle reduces to 2")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 3, 0));
    intervals.push_back(mk(1, 4, 1));
    intervals.push_back(mk(2, 5, 2));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 2);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Disconnected forest stays whole")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 2, 0));
    intervals.push_back(mk(1, 3, 1));
    intervals.push_back(mk(4, 6, 2));
    intervals.push_back(mk(5, 7, 3));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 4);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Triangle plus edge")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 3, 0));
    intervals.push_back(mk(1, 4, 1));
    intervals.push_back(mk(2, 5, 2));
    intervals.push_back(mk(6, 8, 3));
    intervals.push_back(mk(7, 9, 4));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(expected == 4);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Mixed nested + overlaps")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(1, 6, 0));
    intervals.push_back(mk(2, 5, 1));
    intervals.push_back(mk(0, 4, 2));
    intervals.push_back(mk(3, 7, 3));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Left and right dummies right dummy first")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 1, 0));
    intervals.push_back(mk(2, 5, 1));
    intervals.push_back(mk(3, 4, 2));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Left and right dummies left dummy first")
{
    std::vector<cgtd::Interval> intervals;
    intervals.push_back(mk(0, 3, 0));
    intervals.push_back(mk(1, 2, 1));
    intervals.push_back(mk(4, 5, 2));
    validateEndpoints(intervals);

    const int expected = bruteForceMifSize(intervals);
    CHECK(maximumInducedForestOn5Size(intervals) == expected);
}

TEST_CASE("[MaximumInducedForestOn5] Random small instances match brute force (n<=9)")
{
    std::mt19937 rng(1234567);
    for (int trial = 0; trial < 1000; ++trial)
    {
        const int n = 10 + (rng() % 5);
        std::vector<int> perm(2 * n);
        std::iota(perm.begin(), perm.end(), 0);
        std::shuffle(perm.begin(), perm.end(), rng);

        auto intervals = makeIntervalsFromPermutation(perm);
        for (int i = 0; i < n; ++i)
        {
            intervals[i].Index = i;
        }
        validateEndpoints(intervals);

        const int expected = bruteForceMifSize(intervals);
        CHECK(maximumInducedForestOn5Size(intervals) == expected);
    }
}

