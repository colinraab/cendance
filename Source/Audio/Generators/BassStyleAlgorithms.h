#pragma once

#include "GenerativeAlgorithm.h"

class Sub808Bass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "Sub808"; }
};

class UKGarageBass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "UKGarage"; }
};

class TumbaoBass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "Tumbao"; }
};

class DubPedalBass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "DubPedal"; }
};

class ReesePulseBass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "ReesePulse"; }
};

class MotifBass : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); motifIndex_ = 0; }
    juce::String getName() const override { return "MotifBass"; }

private:
    int motifIndex_ = 0;
};

class AcidTripletBass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "AcidTriplet"; }
};

class GlideCounterBass : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); step_ = 0; }
    juce::String getName() const override { return "GlideCounter"; }

private:
    int step_ = 0;
};

class PulseChopBass : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "PulseChop"; }
};

class OctaveBounceBass : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); octaveFlip_ = false; }
    juce::String getName() const override { return "OctaveBounce"; }

private:
    bool octaveFlip_ = false;
};

#define DECLARE_BASS_ALGORITHM(ClassName, DisplayName) \
class ClassName : public GenerativeAlgorithm { \
public: \
    void processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int rootNote, float density, float complexity) override; \
    void reset() override { clearPendingNoteOffs(); step_ = 0; } \
    juce::String getName() const override { return DisplayName; } \
private: \
    int step_ = 0; \
};

DECLARE_BASS_ALGORITHM(ReggaetonSubBass, "ReggaetonSub")
DECLARE_BASS_ALGORITHM(ElectroFunkBass, "ElectroFunk")
DECLARE_BASS_ALGORITHM(MinimalDroneBass, "MinimalDrone")
DECLARE_BASS_ALGORITHM(BrokenOctaveBass, "BrokenOctave")
DECLARE_BASS_ALGORITHM(StepperDubBass, "StepperDub")
DECLARE_BASS_ALGORITHM(FunkPopBass, "FunkPop")
DECLARE_BASS_ALGORITHM(NeuroWobbleBass, "NeuroWobble")
DECLARE_BASS_ALGORITHM(ClaveBass, "ClaveBass")

#undef DECLARE_BASS_ALGORITHM
