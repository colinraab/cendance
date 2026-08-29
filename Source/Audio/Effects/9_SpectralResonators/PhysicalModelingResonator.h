#pragma once

#include "../MasterEffect.h"

#include <array>

class PhysicalModelingResonator : public MasterEffect {
public:
    PhysicalModelingResonator() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setBaseFrequencyHz(float hz);
    void setResonance(float amount);
    void setMix(float amount);

private:
    static constexpr int ResonatorCount = 3;

    void updateDelaySamples();

    bool active = false;
    double sampleRate = 44100.0;
    float baseFrequencyHz = 220.0f;
    float resonance = 0.82f;
    float mix = 0.55f;

    std::array<float, ResonatorCount> frequencyRatios{1.0f, 1.5f, 2.0f};
    std::array<int, ResonatorCount> delaySamples{1, 1, 1};
    std::array<int, ResonatorCount> writePositions{};
    std::array<std::array<float, 2>, ResonatorCount> dampingState{};
    juce::AudioBuffer<float> delayBuffer;
};
