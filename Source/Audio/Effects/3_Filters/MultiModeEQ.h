#pragma once

#include "../MasterEffect.h"

#include <array>

class MultiModeEQ : public MasterEffect {
public:
    MultiModeEQ() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setFrequencyHz(float hz);
    void setShape(float value);
    void setModeSelector(float value);

private:
    enum class Mode : uint8_t {
        HighPass = 0,
        LowPass,
        BandPass,
        Notch,
        Bell,
    };

    void updateCoefficients();

    bool active = false;
    double sampleRate = 44100.0;
    float frequencyHz = 1200.0f;
    float resonanceQ = 0.8f;
    float bellGainDb = 6.0f;
    Mode mode = Mode::LowPass;

    std::array<juce::IIRFilter, 2> filters;
};
