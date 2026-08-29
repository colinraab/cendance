#pragma once
#include "GenerativeAlgorithm.h"

class MarkovMelody : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override;
    juce::String getName() const override { return "Markov"; }

private:
    int currentDegree_ = 0;
};
