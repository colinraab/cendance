#pragma once

#include "../MasterEffect.h"

class Chorus : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setRateHz(float hz);
    void setDepthMs(float ms);
    void setMix(float amount);

private:
    float readDelay(int channel, float delaySamples) const;

    bool active = false;
    double sampleRate = 44100.0;
    float rateHz = 0.55f;
    float depthMs = 8.0f;
    float mix = 0.45f;
    float phase = 0.0f;
    int writePos = 0;
    juce::AudioBuffer<float> delayBuffer;
};
