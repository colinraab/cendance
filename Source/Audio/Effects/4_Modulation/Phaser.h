#pragma once

#include "../MasterEffect.h"

#include <array>

class Phaser : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setRateHz(float hz);
    void setDepth(float amount);
    void setMix(float amount);

private:
    std::array<std::array<float, 4>, 2> z1{};
    bool active = false;
    double sampleRate = 44100.0;
    float rateHz = 0.4f;
    float depth = 0.7f;
    float mix = 0.55f;
    float phase = 0.0f;
};
