#pragma once

#include "GenerativeAlgorithm.h"

class TranceContourLead : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "TranceLead"; }

private:
    int step_ = 0;
};

class RaveStabLead : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "RaveStabs"; }
};

class AfroCallResponseLead : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "CallResp"; }
};

class CinematicSparseLead : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "CineSparse"; }
};

class EuclideanLeadGate : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "EuclidLead"; }
};

class PhraseMutatorLead : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); phraseIndex_ = 0; }
    juce::String getName() const override { return "PhraseMut"; }

private:
    int phraseIndex_ = 0;
};

class GlideRunLead : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); glideStep_ = 0; }
    juce::String getName() const override { return "GlideRun"; }

private:
    int glideStep_ = 0;
};

class MicroMotifLead : public GenerativeAlgorithm {
public:
    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override { clearPendingNoteOffs(); motifCursor_ = 0; }
    juce::String getName() const override { return "MicroMotif"; }

private:
    int motifCursor_ = 0;
};

class WideIntervalLead : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "WideInterval"; }
};

class TripletRushLead : public GenerativeAlgorithm {
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
    juce::String getName() const override { return "TripletRush"; }
};

#define DECLARE_LEAD_ALGORITHM(ClassName, DisplayName) \
class ClassName : public GenerativeAlgorithm { \
public: \
    void processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int rootNote, float density, float complexity) override; \
    void reset() override { clearPendingNoteOffs(); step_ = 0; } \
    juce::String getName() const override { return DisplayName; } \
private: \
    int step_ = 0; \
};

DECLARE_LEAD_ALGORITHM(PentatonicHookLead, "PentatonicHook")
DECLARE_LEAD_ALGORITHM(AcidLineLead, "AcidLine")
DECLARE_LEAD_ALGORITHM(DubEchoLead, "DubEchoLead")
DECLARE_LEAD_ALGORITHM(GarageVoxLead, "GarageVox")
DECLARE_LEAD_ALGORITHM(MinimalPingLead, "MinimalPing")
DECLARE_LEAD_ALGORITHM(OrnamentRunLead, "OrnamentRun")
DECLARE_LEAD_ALGORITHM(SyncopatedPluckLead, "SyncopatedPluck")
DECLARE_LEAD_ALGORITHM(LydianFloatLead, "LydianFloat")

#undef DECLARE_LEAD_ALGORITHM
