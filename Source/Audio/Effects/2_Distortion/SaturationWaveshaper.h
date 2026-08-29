#pragma once

#include "../MasterEffect.h"

class SaturationWaveshaper : public MasterEffect {
public:
    SaturationWaveshaper() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDrive(float amount);
    void setMix(float amount);
    void setInputGainDb(float db);
    void setOutputTrimDb(float db);
    void setBias(float amount);

private:
    bool active = false;
    float drive = 2.0f;
    float mix = 0.5f;
    float inputGainDb = 0.0f;
    float outputTrimDb = 0.0f;
    float bias = 0.0f;
};
