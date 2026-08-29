#pragma once

#include "../MasterEffect.h"

#include <array>

class ErosionDegrade : public MasterEffect {
public:
    ErosionDegrade() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDropProbability(float amount);
    void setHoldMs(float ms);
    void setMix(float amount);

private:
    void updateHoldSamples();

    bool active = false;
    double sampleRate = 44100.0;
    float dropProbability = 0.35f;
    float holdMs = 8.0f;
    float mix = 0.6f;
    int holdSamples = 1;
    uint32_t rngState = 0x9E3779B9u;
    std::array<int, 2> remainingHold{{0, 0}};
    std::array<float, 2> heldValues{{0.0f, 0.0f}};
};
