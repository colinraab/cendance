#pragma once
#include "../MasterEffect.h"
#include <juce_dsp/juce_dsp.h>

class HighPassSweep : public MasterEffect {
public:
    HighPassSweep();
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;
    void setTargetCutoff(float hz);
    void setSweepRate(float amount);

private:
    bool active = false;
    float currentCutoff = 20.0f;
    float targetCutoff = 5000.0f;
    float sweepRate = 0.05f;
    juce::dsp::StateVariableTPTFilter<float> filter;
};
