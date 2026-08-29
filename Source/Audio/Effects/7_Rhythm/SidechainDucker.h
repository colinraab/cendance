#pragma once

#include "../MasterEffect.h"

class SidechainDucker : public MasterEffect {
public:
    SidechainDucker() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;
    void setBpm(float bpm) override;

    void setRepeatDivision(float division);
    void setDepth(float amount);
    void setCurve(float amount);

private:
    void updatePeriodSamples();

    bool active = false;
    double sampleRate = 44100.0;
    float bpm = 120.0f;
    float repeatDivision = 0.25f;
    float depth = 0.75f;
    float curve = 0.5f;
    int periodSamples = 1;
    int phaseSamples = 0;
    float smoothedGain = 1.0f;
    float smoothingCoefficient = 0.0f;
};
