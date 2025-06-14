#include "doctest/doctest.h"
#include "data_structures/chord.h"
#include "data_structures/chord_model.h"
#include "data_structures/distinct_interval_model.h"
#include "data_structures/interval.h"

#include <vector>
#include <iostream>

void checkSameGraph(std::vector<cg::data_structures::Chord>& chords, cg::data_structures::DistinctIntervalModel& intervalModel)
{
    std::vector<cg::data_structures::Interval> intervalIndexToInterval;
    intervalIndexToInterval.reserve(intervalModel.size);
    for(auto i = 0; i < intervalModel.end; ++i)
    {
        const auto& interval = intervalModel.getIntervalByEndpoint(i);
        intervalIndexToInterval[interval.Index] = interval;
    }

    for(auto chord : chords)
    {
        auto interval = intervalIndexToInterval[chord.index()];
        for(auto otherChord : chords)
        {
            auto otherInterval = intervalIndexToInterval[otherChord.index()];

            auto chordsIntersect = chord.intersects(otherChord);
            auto intervalsOverlap = interval.overlaps(otherInterval);
            CHECK(chordsIntersect == intervalsOverlap); // TODO: can i give a proper message
        }
    }
}

TEST_CASE("toDistinctIntervalModel: cycle") 
{
    
    using cg::data_structures::Chord;
    Chord a(0, 1, 0, 1);
    Chord b(1, 2, 1, 1);
    Chord c(2, 3, 2, 1);
    Chord d(3, 4, 3, 1);
    Chord e(4, 5, 4, 1);
    Chord f(5, 0, 5, 1);

    std::vector<Chord> chords = { a, b, c, d, e, f };

    cg::data_structures::ChordModel chordModel(chords);

    auto intervalModel = chordModel.toDistinctIntervalModel();

    checkSameGraph(chords, intervalModel);
}

TEST_CASE("toDistinctIntervalModel: left-and-right sharing") 
{
    
    using cg::data_structures::Chord;
    Chord a(0, 1, 0, 1);
    Chord b(1, 2, 1, 1);
    Chord c(1, 3, 2, 1);
    Chord d(2, 5, 3, 1);
    Chord e(3, 4, 4, 1);
    Chord f(5, 0, 5, 1);

    std::vector<Chord> chords = { a, b, c, d, e, f };

    cg::data_structures::ChordModel chordModel(chords);

    auto intervalModel = chordModel.toDistinctIntervalModel();

    checkSameGraph(chords, intervalModel);
}

TEST_CASE("toDistinctIntervalModel: 4-gap") 
{
    std::vector<cg::data_structures::Chord> chords;
    const int gap = 4;
    const int numChords = 10;
    for(int i = 0; i < numChords; ++i)
    {
        chords.push_back(cg::data_structures::Chord(i, (i + gap) % numChords, i, 1));
    }

    cg::data_structures::ChordModel chordModel(chords);

    auto intervalModel = chordModel.toDistinctIntervalModel();

    checkSameGraph(chords, intervalModel);
}

TEST_CASE("toDistinctIntervalModel: 5-gap + cycle") 
{
    std::vector<cg::data_structures::Chord> chords;
    const int gap = 5;
    const int numEndpoints = 13;
    for(int i = 0; i < numEndpoints; ++i)
    {
        chords.push_back(cg::data_structures::Chord(i, (i + gap) % numEndpoints, chords.size(), 1));
        chords.push_back(cg::data_structures::Chord(i, (i + 1) % numEndpoints, chords.size(), 1));
    }

    cg::data_structures::ChordModel chordModel(chords);

    auto intervalModel = chordModel.toDistinctIntervalModel();

    checkSameGraph(chords, intervalModel);
}

TEST_CASE("toDistinctIntervalModel: 13-circulant") 
{
    std::vector<cg::data_structures::Chord> chords;
    const int numEndpoints = 13;
    const int maxGap = numEndpoints / 2;
    for(int i = 0; i < numEndpoints; ++i)
    {
        for(auto gap = 1; gap <= maxGap; ++gap)
        {
            chords.push_back(cg::data_structures::Chord(i, (i + gap) % numEndpoints, chords.size(), 1));
        }
    }

    cg::data_structures::ChordModel chordModel(chords);

    auto intervalModel = chordModel.toDistinctIntervalModel();

    checkSameGraph(chords, intervalModel);
}
