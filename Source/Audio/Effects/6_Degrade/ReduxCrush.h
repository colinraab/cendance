#pragma once

#include "../MasterEffect.h"

class ReduxCrush : public MasterEffect {
public:
    ReduxCrush() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setBitDepth(float bits);
    void setDownsampleFactor(float factor);
    void setMix(float amount);

private:
    float quantize(float input) const;

    bool active = false;
    float bitDepth = 12.0f;
    int downsampleFactor = 4;
    int downsampleCounter = 0;
    float mix = 1.0f;
    float heldSample[2]{0.0f, 0.0f};
};
