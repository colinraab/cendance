#pragma once

#include "../MasterEffect.h"

class TapeDelay : public MasterEffect {
public:
    TapeDelay() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDelayMs(float ms);
    void setFeedback(float amount);
    void setSaturation(float amount);
    void setWowRate(float rateHz);
    void setWowDepth(float depth);
    void setMix(float amount);

private:
    void updateDelaySamples();
    float saturate(float x);

    bool active = false;
    double sampleRate = 44100.0;
    float delayMs = 350.0f;
    float feedback = 0.45f;
    float saturation = 2.0f;
    float wowRate = 0.4f;
    float wowDepth = 0.15f;
    float mix = 0.35f;
    int delaySamples = 1;
    int writePos = 0;
    float wowPhase = 0.0f;
    float hfState[2] = { 0.0f, 0.0f };
    float hfCoeff = 0.3f;
    juce::AudioBuffer<float> delayBuffer;
};
