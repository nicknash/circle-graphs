#include "doctest/doctest.h"
#include "utils/spinrad_prime.h"
#include "data_structures/graph.h"

TEST_CASE("SpinradPrime reports no split on 5-cycle")
{
    cg::data_structures::Graph g(5);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 0);

    cg::utils::SpinradPrime sp;
    CHECK_FALSE(sp.trySplit(g).has_value());
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

