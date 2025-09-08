#include "doctest/doctest.h"
#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

TEST_CASE("DistinctIntervalModel endpoint iteration")
{
    using cg::data_structures::Interval;
    std::vector<Interval> intervals{
        Interval{0,3,0,1},
        Interval{1,4,1,1},
        Interval{2,5,2,1},
    };
    cg::data_structures::DistinctIntervalModel model(intervals);

    std::vector<int> expectedRight{3,4,5};
    std::vector<int> expectedRightDesc{5,4,3};
    std::vector<int> expectedLeft{0,1,2};
    std::vector<int> expectedLeftDesc{2,1,0};

    {
        size_t i=0;
        for(int r : model.rightEndpoints())
        {
            REQUIRE(i < expectedRight.size());
            CHECK_EQ(r, expectedRight[i]);
            ++i;
        }
        CHECK_EQ(i, expectedRight.size());
    }

    {
        size_t i=0;
        for(int r : model.rightEndpointsDescending())
        {
            REQUIRE(i < expectedRightDesc.size());
            CHECK_EQ(r, expectedRightDesc[i]);
            ++i;
        }
        CHECK_EQ(i, expectedRightDesc.size());
    }

    {
        size_t i=0;
        for(int l : model.leftEndpoints())
        {
            REQUIRE(i < expectedLeft.size());
            CHECK_EQ(l, expectedLeft[i]);
            ++i;
        }
        CHECK_EQ(i, expectedLeft.size());
    }

    {
        size_t i=0;
        for(int l : model.leftEndpointsDescending())
        {
            REQUIRE(i < expectedLeftDesc.size());
            CHECK_EQ(l, expectedLeftDesc[i]);
            ++i;
        }
        CHECK_EQ(i, expectedLeftDesc.size());
    }
}
