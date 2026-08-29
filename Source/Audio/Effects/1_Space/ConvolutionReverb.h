#pragma once

#include "../MasterEffect.h"

#include <juce_dsp/juce_dsp.h>

#include <string_view>

class ConvolutionReverb : public MasterEffect {
public:
    ConvolutionReverb() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setMix(float amount);
    void setPreDelayMs(float ms);
    void setIrGain(float gainDb);
    bool loadIrFromFile(const juce::String& filePath);
    bool loadIrFromResource(std::string_view resourceName);
    int getCurrentIrSize() const;

private:
    void updatePreDelay();

    bool active = false;
    double sampleRate = 44100.0;
    int blockSize = 512;
    float mix = 0.35f;
    float preDelayMs = 0.0f;
    float irGain = 0.0f;
    int preDelaySamples = 0;
    int preWritePos = 0;
    juce::AudioBuffer<float> preDelayBuffer;
    juce::AudioBuffer<float> wetBuffer;
    juce::dsp::Convolution convolution;
    bool irLoaded = false;
};
