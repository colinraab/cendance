#pragma once

#include "../MasterEffect.h"

class TimeFreezer : public MasterEffect {
public:
    TimeFreezer() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setFreezeWindowMs(float ms);
    void setPlaybackRate(float rate);
    void setMix(float amount);

private:
    void updateFreezeWindowSamples();

    bool active = false;
    bool hasSnapshot = false;
    double sampleRate = 44100.0;
    float freezeWindowMs = 320.0f;
    float playbackRate = 1.0f;
    float mix = 0.70f;

    int freezeWindowSamples = 1;
    int writePos = 0;
    float readPos = 0.0f;
    juce::AudioBuffer<float> freezeBuffer;
};
