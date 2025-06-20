#include "doctest/doctest.h"
#include "utils/spinrad_prime.h"
#include "data_structures/graph.h"

#include <iostream>

TEST_CASE("SpinradPrime reports no split on cycles")
{
    for (auto n : {5, 10, 15, 20, 25})
    {
        cg::data_structures::Graph g(n);
        for(auto i = 0; i < n - 1; ++i)
        {
            g.addEdge(i, i + 1);
        }
        g.addEdge(n - 1, 0);

        cg::utils::SpinradPrime sp;
        CHECK_FALSE(sp.trySplit(g).has_value());
    }
}

TEST_CASE("SpinradPrime finds split on 4-cycle")
{
    cg::data_structures::Graph g(4);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 0);

    cg::utils::SpinradPrime sp;
    auto res = sp.trySplit(g);
    CHECK(res.has_value());
    auto [v1, v2] = *res;
    CHECK_NOTHROW(sp.verifySplit(g, v1, v2));
}

TEST_CASE("SpinradPrime finds split on stars")
{
    for (auto n : {5, 10, 15, 20, 25})
    {
        cg::data_structures::Graph g(n);
        for(auto i = 1; i < n; ++i)
        {
            g.addEdge(0, i);
        }
        cg::utils::SpinradPrime sp;
        auto res = sp.trySplit(g);
        CHECK(res.has_value());
        auto [v1, v2] = *res;
        CHECK_NOTHROW(sp.verifySplit(g, v1, v2));
    }
}

TEST_CASE("SpinradPrime finds split on complete graphs")
{
    for (auto n : {5, 10, 15, 20, 25, 50, 100})
    {
        cg::data_structures::Graph g(n);
        for(auto i = 0; i < n; ++i)
        {
            for(auto j = i + 1; j < n; ++j)
            {
                g.addEdge(i, j);
            }
        }
        cg::utils::SpinradPrime sp;
        auto res = sp.trySplit(g);
        CHECK(res.has_value());
        auto [v1, v2] = *res;
        CHECK_NOTHROW(sp.verifySplit(g, v1, v2));
    }
}


TEST_CASE("SpinradPrime does not finds split on join of cycles with one edge missing")
{
    for (auto n : {5, 10, 15, 20, 25, 50, 100})
    {
        // Create two disjoint cycles first
        cg::data_structures::Graph g(2 * n);
        for(auto i = 0; i < n - 1; ++i)
        {
            g.addEdge(i, i + 1);
            g.addEdge(n + i, n + i + 1);
        }
        g.addEdge(n - 1, 0);
        g.addEdge(2 * n - 1, n);

        for(auto i = 0; i < n - 1; ++i) // Connect all but one of the first cycle nodes to the second cycle 
        {
            for(auto j = n; j < 2 * n; ++j)
            {
                g.addEdge(i, j);
            }
        }
        for(auto j = n; j < 2 * n - 1; ++j) // For the last vertex (number n - 1) of the first cycle, only connect it to n - 1 of the second cycle 
        {
            g.addEdge(n - 1, j);
        }

        cg::utils::SpinradPrime sp;
        auto res = sp.trySplit(g);
        CHECK_FALSE(sp.trySplit(g).has_value());
    }
}

TEST_CASE("SpinradPrime does find split on join of cycles")
{
    for (auto n : {5, 10, 15, 20, 25, 50, 100})
    {
        // Create two disjoint cycles first
        cg::data_structures::Graph g(2 * n);
        for(auto i = 0; i < n - 1; ++i)
        {
            g.addEdge(i, i + 1);
            g.addEdge(n + i, n + i + 1);
        }
        g.addEdge(n - 1, 0);
        g.addEdge(2 * n - 1, n);

        for(auto i = 0; i < n; ++i) // Connect every vertex of the first cycle to every vertex of the second cycle
        {
            for(auto j = n; j < 2 * n; ++j)
            {
                g.addEdge(i, j);
            }
        }
        
        cg::utils::SpinradPrime sp;
        auto res = sp.trySplit(g);
        CHECK(res.has_value());
        auto [v1, v2] = *res;
        CHECK_NOTHROW(sp.verifySplit(g, v1, v2));
    }
}
