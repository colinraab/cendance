#pragma once

#include "GenerativeAlgorithm.h"

class HousePianoStabs : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "HouseStabs"; }
};

class AmbientPadSwells : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "AmbientPad"; }
};

class NeoSoulVoicings : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "NeoSoul"; }
};

class TranceGateChords : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "TranceGate"; }
};

class QuartalComping : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "QuartalComp"; }
};

class VoiceCloudChords : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "VoiceCloud"; }
};

class GospelLiftChords : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "GospelLift"; }
};

class DetuneStackChords : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "DetuneStack"; }
};

class BrokenStrumChords : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); strumOffset_ = 0; }
    juce::String getName() const override { return "BrokenStrum"; }

private:
    int strumOffset_ = 0;
};

class PulseClusterChords : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); }
    juce::String getName() const override { return "PulseCluster"; }
};

#define DECLARE_CHORD_ALGORITHM(ClassName, DisplayName) \
class ClassName : public GenerativeAlgorithm { \
public: \
    void processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int rootNote, float density, float complexity) override; \
    void reset() override { clearPendingNoteOffs(); step_ = 0; } \
    juce::String getName() const override { return DisplayName; } \
private: \
    int step_ = 0; \
};

DECLARE_CHORD_ALGORITHM(DubSkankChords, "DubSkanks")
DECLARE_CHORD_ALGORITHM(MinimalPluckChords, "MinimalPlucks")
DECLARE_CHORD_ALGORITHM(RNBKeyChords, "RNBKeys")
DECLARE_CHORD_ALGORITHM(SuspendedPadChords, "SuspendedPad")
DECLARE_CHORD_ALGORITHM(CinematicHitChords, "CinematicHits")
DECLARE_CHORD_ALGORITHM(FifthDroneChords, "FifthDrones")
DECLARE_CHORD_ALGORITHM(GarageOrganChords, "GarageOrgan")
DECLARE_CHORD_ALGORITHM(PolychordChords, "Polychord")

#undef DECLARE_CHORD_ALGORITHM
