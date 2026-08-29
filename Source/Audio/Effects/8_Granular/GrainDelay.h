#pragma once

#include "../MasterEffect.h"

class GrainDelay : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setGrainMs(float ms);
    void setScatter(float amount);
    void setMix(float amount);

private:
    uint32_t nextRandom();

    bool active = false;
    double sampleRate = 44100.0;
    float grainMs = 120.0f;
    float scatter = 0.45f;
    float mix = 0.50f;
    int writePos = 0;
    int grainCounter = 0;
    int currentDelay = 1;
    int previousDelay = 1;
    int delayCrossfadeRemaining = 0;
    int delayCrossfadeSamples = 1;
    uint32_t randomState = 0x12345678u;
    juce::AudioBuffer<float> delayBuffer;
};
