#pragma once

#include "GenerativeAlgorithm.h"

class DnBBreaks : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "DnBBreaks"; }
};

class AfroClaveGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "AfroClave"; }
};

class HouseShuffleGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "HouseShuffle"; }
};

class TrapHalfTimeGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "TrapHalfTime"; }
};

class GlitchPulseGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "GlitchPulse"; }
};

class TechnoRumbleGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "TechnoRumble"; }
};

class JerseyClubGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "JerseyClub"; }
};

class BrokenStepperGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "BrokenStepper"; }
};

class PolyrhythmTomGroove : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override {}
    juce::String getName() const override { return "PolyrhythmToms"; }
};

#define DECLARE_DRUM_ALGORITHM(ClassName, DisplayName) \
class ClassName : public GenerativeAlgorithm { \
public: \
    void processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int rootNote, float density, float complexity) override; \
    void reset() override {} \
    juce::String getName() const override { return DisplayName; } \
};

DECLARE_DRUM_ALGORITHM(ElectroBreaksGroove, "ElectroBreaks")
DECLARE_DRUM_ALGORITHM(GarageSwingGroove, "GarageSwing")
DECLARE_DRUM_ALGORITHM(LatinPercGroove, "LatinPerc")
DECLARE_DRUM_ALGORITHM(MinimalClicksGroove, "MinimalClicks")
DECLARE_DRUM_ALGORITHM(DubSkankGroove, "DubSkank")
DECLARE_DRUM_ALGORITHM(Footwork160Groove, "Footwork160")
DECLARE_DRUM_ALGORITHM(HalfstepGroove, "Halfstep")
DECLARE_DRUM_ALGORITHM(IndustrialGroove, "Industrial")

#undef DECLARE_DRUM_ALGORITHM
