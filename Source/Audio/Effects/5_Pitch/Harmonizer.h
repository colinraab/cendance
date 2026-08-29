#pragma once

#include "PitchShifter.h"

class Harmonizer : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setIntervalSemitones(float value);
    void setBlend(float amount);
    void setMix(float amount);

private:
    PitchShifter voiceA;
    PitchShifter voiceB;
    bool active = false;
    float interval = 7.0f;
    float blend = 0.5f;
    float mix = 0.45f;
    juce::AudioBuffer<float> workBuffer;
};
