#pragma once

#include "../MasterEffect.h"

class Flanger : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setRateHz(float hz);
    void setFeedback(float amount);
    void setMix(float amount);

private:
    float readDelay(int channel, float delaySamples) const;

    bool active = false;
    double sampleRate = 44100.0;
    float rateHz = 0.35f;
    float feedback = 0.45f;
    float mix = 0.55f;
    float phase = 0.0f;
    int writePos = 0;
    juce::AudioBuffer<float> delayBuffer;
};
