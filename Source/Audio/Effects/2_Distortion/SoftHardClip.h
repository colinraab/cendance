#pragma once

#include "../MasterEffect.h"

class SoftHardClip : public MasterEffect {
public:
    SoftHardClip() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDrive(float amount);
    void setHardness(float amount);
    void setMix(float amount);
    void setInputGainDb(float db);
    void setOutputGainDb(float db);

private:
    bool active = false;
    float drive = 2.0f;
    float hardness = 0.5f;
    float mix = 0.6f;
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
};
