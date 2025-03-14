#pragma once


#include <vector>
#include <stack>
#include <optional>
#include <span>
#include <format>
#include <stdexcept>

class Interval;

namespace cg::data_structures
{
    class SimpleIntervalRep
    {
        std::vector<Interval> intervals;
    public:
        const int End;
        const int Size;

        SimpleIntervalRep(std::span<const Interval> intervals);

        [[nodiscard]] std::optional<Interval> tryGetIntervalByRightEndpoint(int maybeRightEndpoint) const;
        [[nodiscard]] std::optional<Interval> tryGetIntervalByLeftEndpoint(int maybeLeftEndpoint) const;
    };
}

class Valiente
{
public:
    static void computeMIS(const SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.End + 1, 0);
        std::vector<int> CMIS(intervals.Size, 0);
        for(auto i = 0; i < intervals.End; ++i)
        {
            auto maybeOuterInterval = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeOuterInterval)
            {
                auto outerInterval = maybeOuterInterval.value();
                for(auto j = outerInterval.Right - 1; j > outerInterval.Left; --j)
                {
                    auto maybeInnerInterval = intervals.tryGetIntervalByLeftEndpoint(j);
                    if(maybeInnerInterval)
                    {
                        auto innerInterval = maybeInnerInterval.value();
                        auto candidate = MIS[innerInterval.Right + 1] + CMIS[innerInterval.Index];
                        if(innerInterval.Right < outerInterval.Right && // Strictly speaking, this bounds check could be removed because CMIS and MIS on the previous line
                                                                        // will both be zero when it is false, but it's a bit confusing to write the code that way. 
                           candidate > MIS[innerInterval.Left + 1])
                        {
                            MIS[j] = candidate;
                        }
                    }
                    else
                    {
                        MIS[j] = MIS[j + 1];
                    }
 
                }
                CMIS[outerInterval.Index] = 1 + MIS[outerInterval.Left + 1];
            }
        }
    }
};

class Switching
{
public:
    static void computeMIS(const SimpleIntervalRep& intervals)
    {
        // TODO: Compute density
        int density = 12345;
        if(!PureOutputSensitive::tryComputeMIS(intervals, density))
        {
            Valiente::computeMIS(intervals);
        }
    }
};

class NaiveLinearSpaceQuadraticTime
{
public:
    static void computeMIS(const SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.End + 1, 0);
        std::vector<int> CMIS(intervals.Size, 0);
        for(auto i = 0; i < intervals.End; ++i)
        {
            auto maybeIntervalRight = intervals.tryGetIntervalByRightEndpoint(i);
            if(maybeIntervalRight)
            {
                auto interval = maybeIntervalRight.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
            }
            for (auto j = i - 1; j >= 0; --j)
            {
                MIS[j] = MIS[j + 1];
                auto maybeIntervalLeft = intervals.tryGetIntervalByLeftEndpoint(j);
                if(maybeIntervalLeft)
                {
                    auto interval = maybeIntervalLeft.value();
                    if(interval.Right <= i)
                    {
                        MIS[j] = std::max(MIS[j + 1], 1 + CMIS[interval.Index] + MIS[interval.Right + 1]);
                    }
                } 
            }
        }
    }
};