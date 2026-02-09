#include "doctest/doctest.h"
#include "utils/spinrad_prime.h"
#include "data_structures/graph.h"
#include "data_structures/interval.h"
#include "utils/interval_model_utils.h"
#include "data_structures/distinct_interval_model.h"
#include "mis/distinct/combined_output_sensitive.h"
#include "utils/counters.h"

#include <iostream>
#include <format>

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

TEST_CASE("SpinradPrime finds nested prime intervals are indeed prime")
{
    auto intervals = cg::interval_model_utils::generatePrimeNestedIntervals(50);
    cg::data_structures::Graph g(intervals.size());

    for (int r = 0; r < intervals.size(); ++r)
    {
        auto &p = intervals[r];
        for (int j = r + 1; j < intervals.size(); ++j)
        {
            if (p.overlaps(intervals[j]))
            {
                g.addEdge(r, j);
            }
        }
    }

    cg::utils::SpinradPrime sp;
    auto res = sp.trySplit(g);
    CHECK_FALSE(sp.trySplit(g).has_value());
}

TEST_CASE("SpinradPrime expt")
{
    cg::data_structures::Graph g(10);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 6);

    g.addEdge(1, 6);
    g.addEdge(1, 7);

    g.addEdge(2, 6);

    g.addEdge(3, 4);
    g.addEdge(3, 5);
    g.addEdge(3, 6);
    g.addEdge(3, 8);

    g.addEdge(4, 7);
    g.addEdge(4, 8);
    g.addEdge(4, 9);

    g.addEdge(5, 8);

    g.addEdge(6, 9);

    cg::utils::SpinradPrime sp;
    /*auto res = sp.trySplit(g);
    auto [v1, v2] = *res;
    for(auto v : v1)
    {
        std::cout << v << std::endl;
    }*/
    
    CHECK_FALSE(sp.trySplit(g).has_value());
}

TEST_CASE("SpinradPrime expt2")
{
    cg::data_structures::Graph g(16);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 9);

    g.addEdge(1, 9);
    g.addEdge(1, 10);

    g.addEdge(2, 9);

    g.addEdge(3, 4);
    g.addEdge(3, 5);
    g.addEdge(3, 9);
    g.addEdge(3, 11);

    g.addEdge(4, 10);
    g.addEdge(4, 11);
    g.addEdge(4, 12);
    g.addEdge(4, 9);

    g.addEdge(5, 11);
    //g.addEdge(5, 12);

    g.addEdge(6, 7);
    g.addEdge(6, 8);
    g.addEdge(6, 9);
    g.addEdge(6, 11);
    g.addEdge(6, 13);

    g.addEdge(7, 12);
    g.addEdge(7, 13);
    g.addEdge(7, 14);

    g.addEdge(8, 13);

    g.addEdge(9, 11);
    g.addEdge(9, 13);
    g.addEdge(9, 14);

    g.addEdge(11, 13);
    
    
    g.addEdge(15, 11);
    g.addEdge(15, 13);
    g.addEdge(15, 14);




    cg::utils::SpinradPrime sp;
    auto res = sp.trySplit(g);
    CHECK_FALSE(res.has_value());

    /*auto [v1, v2] = *res;
    for(auto v : v1)
    {
        std::cout << v << std::endl;
    }*/
    CHECK_FALSE(sp.trySplit(g).has_value());
}
