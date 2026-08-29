#pragma once

#include "../MasterEffect.h"

#include <array>

class CloudGenerator : public MasterEffect {
public:
    CloudGenerator() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setGrainSizeMs(float ms);
    void setDensity(float grainsPerSecond);
    void setPitchScatter(float semitones);
    void setPosition(float position);
    void setSpray(float ms);
    void setMix(float amount);

private:
    struct Grain {
        bool active = false;
        int startSample = 0;
        int lengthSamples = 0;
        int positionSamples = 0;
        float pitchRatio = 1.0f;
        float panL = 0.707f;
        float panR = 0.707f;
        float amplitude = 1.0f;
    };

    static constexpr int maxGrains = 32;
    uint32_t nextRandom();

    bool active = false;
    double sampleRate = 44100.0;
    float grainSizeMs = 120.0f;
    float density = 8.0f;
    float pitchScatter = 0.0f;
    float position = 0.0f;
    float spray = 30.0f;
    float mix = 0.50f;
    int writePos = 0;
    uint32_t randomState = 0x12345678u;
    int samplesUntilNextGrain = 0;
    std::array<Grain, maxGrains> grains;
    juce::AudioBuffer<float> cloudBuffer;
    juce::AudioBuffer<float> delayBuffer;
};
