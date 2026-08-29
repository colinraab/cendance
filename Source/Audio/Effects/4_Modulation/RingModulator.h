#pragma once

#include "../MasterEffect.h"

class RingModulator : public MasterEffect {
public:
    RingModulator() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setRateHz(float hz);
    void setDepth(float amount);
    void setMix(float amount);

private:
    bool active = false;
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float rateHz = 25.0f;
    float depth = 0.7f;
    float mix = 0.75f;
};
