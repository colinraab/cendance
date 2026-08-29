#pragma once

#include "../MasterEffect.h"

#include <array>

class FrequencyShifter : public MasterEffect {
public:
    FrequencyShifter() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setShiftHz(float hz);
    void setMix(float amount);
    void setStereoPhase(float amount);

private:
    struct AllPassState {
        float x1 = 0.0f;
        float y1 = 0.0f;
    };

    static constexpr int HilbertStageCount = 4;
    using HilbertStates = std::array<AllPassState, HilbertStageCount>;

    static float processAllPass(float input, float coeff, AllPassState& state);
    static float processChain(float input,
                              const std::array<float, HilbertStageCount>& coeffs,
                              HilbertStates& states);

    bool active = false;
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float shiftHz = 140.0f;
    float mix = 0.65f;
    float stereoPhase = 0.5f;

    std::array<HilbertStates, 2> iStates{};
    std::array<HilbertStates, 2> qStates{};

    static constexpr std::array<float, HilbertStageCount> kHilbertICoeffs{
        0.4794008656f,
        0.8762184935f,
        0.9765975895f,
        0.9974992559f,
    };

    static constexpr std::array<float, HilbertStageCount> kHilbertQCoeffs{
        0.1617584983f,
        0.7330289323f,
        0.9453497003f,
        0.9905991567f,
    };
};
