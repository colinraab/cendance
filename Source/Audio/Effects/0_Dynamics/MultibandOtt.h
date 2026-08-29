#pragma once

#include "../MasterEffect.h"

#include <array>

class MultibandOtt : public MasterEffect {
public:
    MultibandOtt() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setLowThresholdDb(float db);
    void setMidThresholdDb(float db);
    void setHighThresholdDb(float db);
    void setLowRatio(float ratio);
    void setMidRatio(float ratio);
    void setHighRatio(float ratio);
    void setMakeupDb(float db);
    void setMix(float mix);

private:
    void updateCoefficients();
    void splitBands(const float* input, int numSamples);
    void processBand(int bandIndex, float* data, int numSamples);
    void mergeBands(float* output, int numSamples);

    bool active = false;
    double sampleRate = 44100.0;

    // Crossover frequencies
    float lowCrossoverHz = 200.0f;
    float highCrossoverHz = 4000.0f;

    // Per-band thresholds and ratios
    float lowThresholdDb = -18.0f;
    float midThresholdDb = -18.0f;
    float highThresholdDb = -18.0f;
    float lowRatio = 4.0f;
    float midRatio = 4.0f;
    float highRatio = 4.0f;

    // OTT upward compression amount (0..1)
    float upwardAmount = 0.5f;

    float makeupDb = 0.0f;
    float mix = 1.0f;

    // Crossover filters (2nd-order Linkwitz-Riley = two cascaded Butterworth)
    struct CrossoverFilter {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
    };

    CrossoverFilter lp1, hp1, lp2, hp2;

    // Per-band compression state
    struct BandState {
        float envelope = 0.0f;
        float attackCoeff = 0.0f;
        float releaseCoeff = 0.0f;
    };
    std::array<BandState, 3> bandStates;

    // Working buffers for each band
    juce::AudioBuffer<float> bandBuffer0;
    juce::AudioBuffer<float> bandBuffer1;
    juce::AudioBuffer<float> bandBuffer2;
    juce::AudioBuffer<float> tempBuffer;
    juce::AudioBuffer<float> splitBuffer; // temp for crossover splitting
};
