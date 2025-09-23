#pragma once

#include <span>

#include "data_structures/interval.h"
#include "data_structures/distinct_interval_model.h"

namespace cg::data_structures
{
    class DistinctIntervalModel;
    class Interval;
}


namespace cg::mif
{
template <class T>
    struct array4
    {
        std::size_t n;
        std::vector<T> data;
        array4() = default;
        explicit array4(std::size_t n, const T& empty) : n(n), data(n * n * n * n, empty) {}
        T &operator()(std::size_t i, std::size_t j, std::size_t k, std::size_t l)
        {
            return data[((i * n + j) * n + k) * n + l];
        }
        const T &operator()(std::size_t i, std::size_t j, std::size_t k, std::size_t l) const
        {
            return data[((i * n + j) * n + k) * n + l];
        }
    };

    template <class T>
    struct array3
    {
        std::size_t n;
        std::vector<T> data;
        explicit array3(std::size_t n, const T& empty) : n(n), data(n * n * n, empty) {}
        T &operator()(std::size_t i, std::size_t j, std::size_t k)
        {
            return data[(i * n + j) * n + k];
        }
        const T &operator()(std::size_t i, std::size_t j, std::size_t k) const
        {
            return data[(i * n + j) * n + k];
        }
    };

        enum ChildType
        {
            Undefined,
            None,
            Real,
            Dummy
        };
inline std::string_view to_string(ChildType t) {
    switch (t) {
        case ChildType::Undefined: return "Undefined";
        case ChildType::None:      return "None";
        case ChildType::Dummy:     return "Dummy";
        case ChildType::Real:      return "Real";
    }
    return "???";
}



    // This is Gavril's algorithm for the maximum induced forest of a circle graph:
    // "Minimum weight feedback vertex sets in circle graphs", Information Processing Letters 107 (2008),pp1-6
    class Gavril
    {
    public:
        static const int Invalid = -1;

        struct ForestScore
        {
            int score;
            int childIntervalIdx;
        };
        struct DummyForestScore
        {
            int score;
            int childIntervalIdx;
        };
        struct ChildChoice
        {
            ChildType childType;
            int innerScore;
            int qPrime;
            int xPrime;
            int childIntervalIdx;
        };
        struct Forests
        {
            array4<ForestScore> leftForestScores;
            array3<DummyForestScore> dummyLeftForestScores;
            array4<ForestScore> rightForestScores;
            array3<DummyForestScore> dummyRightForestScores;
        };
        struct ChildChoices
        {
            array4<ChildChoice> leftChildChoices;
            array4<ChildChoice> rightChildChoices;
        };

        static void computeRightForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<ForestScore>& rightForestScores, array3<DummyForestScore>& dummyRightForestScores, array4<ChildChoice>& rightChildChoices);
        static void computeRightForests(int layerIdx, const std::vector<cg::data_structures::Interval>& cumulativeIntervals, Forests& forests, array4<ChildChoice>& rightChildChoices);
        static void computeNewIntervalRightForests(int layerIdx, const std::vector<cg::data_structures::Interval>& newIntervalsAtThisLayer, const std::vector<cg::data_structures::Interval>& allIntervalsBeforeThisLayer, Forests& forests, array4<ChildChoice>& rightChildChoices);
        static void computeRightChildChoices(const Forests &forests, const std::vector<cg::data_structures::Interval> allIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervalsOneBehind, array4<ChildChoice> &rightChildChoices, int layerIdx);

        static void computeLeftForestBaseCase(const std::vector<cg::data_structures::Interval>& firstLayerIntervals, array4<ForestScore>& leftForestScores, array4<ChildChoice>& leftChildChoices);
        static void computeLeftForests(int layerIdx, const std::vector<cg::data_structures::Interval>& cumulativeIntervals, Forests& forests, array4<ChildChoice>& leftChildChoices);
        static void computeNewIntervalLeftForests(int layerIdx, const std::vector<cg::data_structures::Interval>& newIntervalsAtThisLayer, const std::vector<cg::data_structures::Interval>& allIntervalsBeforeThisLayer, Forests& forests, array4<ChildChoice>& leftChildChoices);
        static void computeLeftChildChoices(const Forests &forests, const std::vector<cg::data_structures::Interval> allIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervals, const std::vector<cg::data_structures::Interval> &cumulativeIntervalsOneBehind, array4<ChildChoice> &leftChildChoices, int layerIdx);
      
        static std::vector<int> constructMif(const cg::data_structures::DistinctIntervalModel& intervalModel, int numLayers, const Forests& forests, const ChildChoices& innerChoices);
        static std::vector<cg::data_structures::Interval> computeMif(std::span<const cg::data_structures::Interval> intervals);
    };
}

template<>
struct std::formatter<cg::mif::ChildType> : std::formatter<std::string_view> {
    // no custom parse — we just reuse the base
    template<class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return std::formatter<std::string_view>::parse(ctx); }

    template<class FormatContext>
    auto format(const cg::mif::ChildType& t, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(cg::mif::to_string(t), ctx);
    }
};
