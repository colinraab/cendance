#pragma once

#include "../MasterEffect.h"

class AsymShaper : public MasterEffect {
public:
    AsymShaper() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDrive(float amount);
    void setAsymmetry(float amount);
    void setMix(float amount);
    void setInputGainDb(float db);
    void setOutputGainDb(float db);

private:
    bool active = false;
    float drive = 2.5f;
    float asymmetry = 0.35f;
    float mix = 0.65f;
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
};
