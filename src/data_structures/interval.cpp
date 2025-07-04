
#include "data_structures/interval.h"

namespace cg::data_structures
{
    Interval::Interval(int left, int right, int index, int weight) : Left(left), Right(right), Index(index), Weight(weight)
    {
        if(left >= right)
        {
            throw std::runtime_error(std::format("Invalid end-points for interval {}. Must have left strictly less than right.", *this));
        }
        if(index < 0)
        {
            throw std::runtime_error(std::format("Invalid index for interval {}. Interval index must be non-negative.", *this));
        }
    }

    [[nodiscard]] int Interval::length() const
    {
        return Right - Left;
    }

    [[nodiscard]] bool Interval::overlaps(const Interval& other) const
    {
        return Index != other.Index && // By convention, make it so an interval doesn't overlap itself.
               (Left <= other.Left && other.Left <= Right && Right <= other.Right ||
               other.Left <= Left && Left <= other.Right && other.Right <= Right);
    }
}