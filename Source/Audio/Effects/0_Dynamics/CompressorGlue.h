#pragma once

#include "../MasterEffect.h"

#include <array>

class CompressorGlue : public MasterEffect {
public:
    CompressorGlue() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setThresholdDb(float db);
    void setRatio(float newRatio);
    void setMakeupDb(float db);

private:
    void updateCoefficients();

    bool active = false;
    double sampleRate = 44100.0;
    float thresholdDb = -18.0f;
    float ratio = 4.0f;
    float makeupDb = 0.0f;
    float attackMs = 8.0f;
    float releaseMs = 80.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    std::array<float, 2> envelope{{0.0f, 0.0f}};
};
