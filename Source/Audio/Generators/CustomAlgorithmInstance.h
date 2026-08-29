#pragma once

#include "GenerativeAlgorithm.h"
#include "../../App/CustomAlgorithmPreset.h"
#include "../../App/AlgorithmPresetRegistry.h"

#include <cstdint>
#include <random>

class CustomAlgorithmInstance : public GenerativeAlgorithm {
public:
    explicit CustomAlgorithmInstance(const CustomAlgorithmPreset& preset);
    ~CustomAlgorithmInstance() override = default;

    void setPreset(CustomAlgorithmPreset preset);
    const CustomAlgorithmPreset& preset() const { return preset_; }

    void processMidi(juce::MidiBuffer& buffer,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples,
                     const Scale& scale,
                     int rootNote,
                     float density,
                     float complexity) override;

    void reset() override;
    juce::String getName() const override;

private:
    CustomAlgorithmPreset preset_;
    uint8_t currentStep_ = 0;
    double stepDurationBeats_ = 0.0;
    mutable std::mt19937 rng_;

    int mapTrackToMidiNote(uint8_t step, int rootNote, const Scale& scale) const;
    static uint8_t getDefaultDrumNote(uint8_t step);
};
