#pragma once

#include "../MasterEffect.h"

class BeatRepeatInsert : public MasterEffect {
public:
    BeatRepeatInsert() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;
    void setBpm(float bpm) override;

    void setRepeatDivision(float division);
    void setMix(float amount);
    void setFeedback(float amount);

private:
    bool active = false;
    double sampleRate = 44100.0;
    float bpm = 120.0f;
    float repeatDivision = 0.25f;
    float mix = 0.8f;
    float feedback = 0.35f;
    juce::AudioBuffer<float> delayBuffer;
    int writePos = 0;
    int readPos = 0;
    int stutterStartIndex = 0;
    int stutterEndIndex = 0;
    int repeatLength = 1;
    int repeatPhase = 0;
    int crossfadeSamples = 1;
    int transitionSamples = 1;
    float wetGain = 0.0f;
    float wetGainStep = 1.0f;
    bool targetActive = false;
};
