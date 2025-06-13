#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "data_structures/interval.h"

TEST_CASE("Interval length and overlap") {
    using cg::data_structures::Interval;
    Interval a(0, 5, 0, 1);
    Interval b(3, 7, 1, 1);
    Interval c(6, 8, 2, 1);

    CHECK(a.length() == 5);
    CHECK(b.length() == 4);
    CHECK(a.overlaps(b));
    CHECK(!a.overlaps(c));
}

TEST_CASE("Interval throws on invalid input") {
    using cg::data_structures::Interval;
    CHECK_THROWS_AS(Interval(5, 5, 0, 1), std::runtime_error);
    CHECK_THROWS_AS(Interval(5, 4, 0, 1), std::runtime_error);
    CHECK_THROWS_AS(Interval(0, 1, -1, 1), std::runtime_error);
}
