#pragma once

#include "../MasterEffect.h"

class CombFilter : public MasterEffect {
public:
    CombFilter() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDelayMs(float ms);
    void setFeedback(float amount);
    void setMix(float amount);

private:
    void updateDelaySamples();

    bool active = false;
    double sampleRate = 44100.0;
    float delayMs = 16.0f;
    float feedback = 0.6f;
    float mix = 0.45f;
    int delaySamples = 1;
    int writePos = 0;
    juce::AudioBuffer<float> delayBuffer;
};
