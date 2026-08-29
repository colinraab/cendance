#pragma once

#include "../MasterEffect.h"

#include <array>

class FormantFilter : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setVowel(float selector);
    void setResonance(float amount);
    void setMix(float amount);

private:
    struct BandState {
        float lp = 0.0f;
        float bp = 0.0f;
    };

    float processBand(float input, int channel, int band, float frequency);

    bool active = false;
    double sampleRate = 44100.0;
    float vowel = 0.0f;
    float resonance = 0.75f;
    float mix = 0.65f;
    std::array<std::array<BandState, 3>, 2> states{};
};
