#pragma once
#include "../MasterEffect.h"
#include <juce_dsp/juce_dsp.h>

class ReverbWash : public MasterEffect {
public:
    ReverbWash();
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;
    void setTargetMix(float mixAmount);
    void setRoomSize(float size);
    void setDamping(float dampingAmount);

private:
    void applyParams();

    bool active = false;
    float sendAmount = 0.0f;
    float targetMix = 1.0f;
    float roomSize = 1.0f;
    float damping = 0.2f;
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> wetBuffer;
};
