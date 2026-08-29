#pragma once

#include "../MasterEffect.h"

#include <array>

class PeakLimiter : public MasterEffect {
public:
    PeakLimiter() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setCeilingDb(float db);
    void setReleaseMs(float ms);

private:
    void updateReleaseCoefficient();

    bool active = false;
    double sampleRate = 44100.0;
    float ceilingDb = -1.0f;
    float releaseMs = 60.0f;
    float ceilingLinear = 0.8912509f;
    float releaseCoeff = 0.0f;
    std::array<float, 2> gain{{1.0f, 1.0f}};
};
