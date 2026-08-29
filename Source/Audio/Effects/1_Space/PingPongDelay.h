#pragma once

#include "../MasterEffect.h"

class PingPongDelay : public MasterEffect {
public:
    PingPongDelay() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDelayMs(float ms);
    void setFeedback(float amount);
    void setMix(float amount);
    void setCrossoverFreq(float freq);

private:
    void updateDelaySamples();
    void updateCrossover();

    bool active = false;
    double sampleRate = 44100.0;
    float delayMs = 280.0f;
    float feedback = 0.40f;
    float mix = 0.35f;
    float crossoverFreq = 2000.0f;
    int delaySamples = 1;
    int writePosL = 0;
    int writePosR = 0;
    float lpStateL = 0.0f, lpStateR = 0.0f;
    float lpCoeff = 0.0f;
    juce::AudioBuffer<float> delayBufferL;
    juce::AudioBuffer<float> delayBufferR;
};
