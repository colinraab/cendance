#pragma once

#include "../MasterEffect.h"

#include <array>

class TransientShaper : public MasterEffect {
public:
    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setAttack(float amount);
    void setSustain(float amount);
    void setMix(float amount);

private:
    void updateCoefficients();

    bool active = false;
    double sampleRate = 44100.0;
    float attack = 0.35f;
    float sustain = 0.0f;
    float mix = 1.0f;
    float fastCoeff = 0.0f;
    float slowCoeff = 0.0f;
    std::array<float, 2> fastEnv{};
    std::array<float, 2> slowEnv{};
};
