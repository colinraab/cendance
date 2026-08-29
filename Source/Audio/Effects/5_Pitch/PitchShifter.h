#pragma once

#include "../MasterEffect.h"

#include <array>

class PitchShifter : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setSemitones(float value);
    void setWindowMs(float ms);
    void setMix(float amount);

private:
    float readDelay(int channel, float position) const;

    bool active = false;
    double sampleRate = 44100.0;
    float semitones = 12.0f;
    float windowMs = 80.0f;
    float mix = 0.55f;
    int writePos = 0;
    float readPhase = 0.0f;
    juce::AudioBuffer<float> delayBuffer;
};
