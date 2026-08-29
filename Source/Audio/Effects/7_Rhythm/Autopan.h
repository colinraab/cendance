#pragma once

#include "../MasterEffect.h"

class Autopan : public MasterEffect {
public:
    Autopan() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setRateHz(float hz);
    void setDepth(float amount);
    void setWidth(float amount);

private:
    bool active = false;
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float rateHz = 0.75f;
    float depth = 0.7f;
    float width = 1.0f;
};
