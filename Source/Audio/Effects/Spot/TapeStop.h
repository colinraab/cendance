#pragma once
#include "../MasterEffect.h"
#include <juce_dsp/juce_dsp.h>

class TapeStop : public MasterEffect {
public:
    TapeStop();
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;
    void setStopTimeSeconds(float seconds);

private:
    void updateSpeedDelta();

    bool active = false;
    double sampleRate = 44100.0;
    float stopTimeSeconds = 0.5f;
    float currentSpeed = 1.0f;
    float speedDelta = 0.0f;
    float delayTimeSamples = 0.0f;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{176400};
};
