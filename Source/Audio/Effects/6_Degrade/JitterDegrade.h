#pragma once

#include "../MasterEffect.h"

class JitterDegrade : public MasterEffect {
public:
    JitterDegrade() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setBaseDelayMs(float ms);
    void setJitterMs(float ms);
    void setMix(float amount);

private:
    void updateDelaySamples();

    bool active = false;
    double sampleRate = 44100.0;
    float baseDelayMs = 10.0f;
    float jitterMs = 6.0f;
    float mix = 0.55f;
    int baseDelaySamples = 1;
    int jitterRangeSamples = 1;
    int writePos = 0;
    uint32_t rngState = 0x12345678u;
    juce::AudioBuffer<float> delayBuffer;
};
