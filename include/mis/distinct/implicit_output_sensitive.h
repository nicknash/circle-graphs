#pragma once

#include <stack>
#include <vector>
#include <optional>

#include <map>
#include <list>
#include <limits>


namespace cg::data_structures
{
    class DistinctIntervalRep;
    class Interval;
}

namespace cg::mis
{
    class IndependentSet;
    class ImplicitIndependentSet;
    class MonotoneSeq;

    class IntervalStore
    {
        std::vector<std::list<cg::data_structures::Interval>> _cmisToIntervals; // This needs to be replaced with an efficient data structure.
        int _maxContainedMIS;
    public:
        IntervalStore(int numIntervals);
        void addInterval(int containedMIS, cg::data_structures::Interval& interval);
        std::optional<cg::data_structures::Interval> tryGetRelevantInterval(int containedMIS, int left, int right);
        int getMaxContainedMIS();
    };
}


namespace cg::mis::distinct
{
    class ImplicitOutputSensitive
    {
    private:
        static void updateAt(std::stack<int> &pendingUpdates, std::vector<int> &MIS, int indexToUpdate, int newMisValue);
        static bool tryUpdate(const cg::data_structures::DistinctIntervalRep &intervals, std::stack<cg::data_structures::Interval> &pendingUpdates, cg::mis::ImplicitIndependentSet& independentSet, const cg::data_structures::Interval &interval, cg::mis::MonotoneSeq &MIS, cg::mis::IntervalStore &intervalStore, int maxAllowedMIS);
    public:
        static std::optional<std::vector<cg::data_structures::Interval>> tryComputeMIS(const cg::data_structures::DistinctIntervalRep &intervals, int maxAllowedMIS);
    };
}
