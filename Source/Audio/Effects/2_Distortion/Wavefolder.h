#pragma once

#include "../MasterEffect.h"

class Wavefolder : public MasterEffect {
public:
    Wavefolder() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDrive(float amount);
    void setFoldAmount(float amount);
    void setMix(float amount);
    void setInputGainDb(float db);
    void setOutputGainDb(float db);

private:
    bool active = false;
    float drive = 3.0f;
    float foldAmount = 0.5f;
    float mix = 0.6f;
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
};
