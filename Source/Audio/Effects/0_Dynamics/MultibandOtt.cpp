#include "MultibandOtt.h"

#include <algorithm>
#include <cmath>

void MultibandOtt::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;

    bandBuffer0.setSize(1, newBlockSize, false, false, true);
    bandBuffer1.setSize(1, newBlockSize, false, false, true);
    bandBuffer2.setSize(1, newBlockSize, false, false, true);
    tempBuffer.setSize(1, newBlockSize, false, false, true);
    splitBuffer.setSize(1, newBlockSize, false, false, true);

    reset();
    updateCoefficients();
}

void MultibandOtt::reset()
{
    for (auto& state : bandStates)
    {
        state.envelope = 0.0f;
        state.attackCoeff = 0.0f;
        state.releaseCoeff = 0.0f;
    }

    lp1 = {};
    hp1 = {};
    lp2 = {};
    hp2 = {};

    bandBuffer0.clear();
    bandBuffer1.clear();
    bandBuffer2.clear();
    tempBuffer.clear();
    splitBuffer.clear();

    updateCoefficients();
}

void MultibandOtt::updateCoefficients()
{
    // Design Linkwitz-Riley crossover filters (two cascaded Butterworth)
    // Low crossover
    {
        const float omega = std::tan(juce::MathConstants<float>::pi * lowCrossoverHz / static_cast<float>(sampleRate));
        const float omega2 = omega * omega;
        const float sqrt2_omega = std::sqrt(2.0f) * omega;
        const float denom = 1.0f + sqrt2_omega + omega2;

        lp1.b0 = omega2 / denom;
        lp1.b1 = 2.0f * lp1.b0;
        lp1.b2 = lp1.b0;
        lp1.a1 = 2.0f * (omega2 - 1.0f) / denom;
        lp1.a2 = (1.0f - sqrt2_omega + omega2) / denom;

        hp1.b0 = 1.0f / denom;
        hp1.b1 = -2.0f * hp1.b0;
        hp1.b2 = hp1.b0;
        hp1.a1 = lp1.a1;
        hp1.a2 = lp1.a2;
    }

    // High crossover
    {
        const float omega = std::tan(juce::MathConstants<float>::pi * highCrossoverHz / static_cast<float>(sampleRate));
        const float omega2 = omega * omega;
        const float sqrt2_omega = std::sqrt(2.0f) * omega;
        const float denom = 1.0f + sqrt2_omega + omega2;

        lp2.b0 = omega2 / denom;
        lp2.b1 = 2.0f * lp2.b0;
        lp2.b2 = lp2.b0;
        lp2.a1 = 2.0f * (omega2 - 1.0f) / denom;
        lp2.a2 = (1.0f - sqrt2_omega + omega2) / denom;

        hp2.b0 = 1.0f / denom;
        hp2.b1 = -2.0f * hp2.b0;
        hp2.b2 = hp2.b0;
        hp2.a1 = lp2.a1;
        hp2.a2 = lp2.a2;
    }

    // Compression coefficients — fast attack, medium release
    const float attackMs = 1.0f;
    const float releaseMs = 50.0f;
    for (auto& state : bandStates)
    {
        state.attackCoeff = std::exp(-1.0f / (attackMs * static_cast<float>(sampleRate) / 1000.0f));
        state.releaseCoeff = std::exp(-1.0f / (releaseMs * static_cast<float>(sampleRate) / 1000.0f));
    }
}

void MultibandOtt::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1)
        return;

    // Mix down to mono for processing
    float* mixData = tempBuffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i)
    {
        float sum = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            sum += buffer.getSample(ch, i);
        mixData[i] = sum / static_cast<float>(numChannels);
    }

    // Split into 3 bands
    splitBands(mixData, numSamples);

    // Process each band
    processBand(0, bandBuffer0.getWritePointer(0), numSamples);
    processBand(1, bandBuffer1.getWritePointer(0), numSamples);
    processBand(2, bandBuffer2.getWritePointer(0), numSamples);

    // Merge bands
    mergeBands(mixData, numSamples);

    // Apply makeup gain and mix back
    const float makeupLinear = std::pow(10.0f, makeupDb / 20.0f);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            const float dry = channelData[i];
            const float wet = mixData[i] * makeupLinear;
            channelData[i] = dry + (wet - dry) * mix;
        }
    }
}

void MultibandOtt::splitBands(const float* input, int numSamples)
{
    float* lowData = bandBuffer0.getWritePointer(0);
    float* midData = bandBuffer1.getWritePointer(0);
    float* highData = bandBuffer2.getWritePointer(0);
    float* hp1Out = splitBuffer.getWritePointer(0);

    for (int i = 0; i < numSamples; ++i)
    {
        const float x = input[i];

        // LP1 (low-pass at low crossover)
        const float y_lp = lp1.b0 * x + lp1.b1 * lp1.x1 + lp1.b2 * lp1.x2
                          - lp1.a1 * lp1.y1 - lp1.a2 * lp1.y2;
        lp1.x2 = lp1.x1; lp1.x1 = x;
        lp1.y2 = lp1.y1; lp1.y1 = y_lp;
        lowData[i] = y_lp;

        // HP1 (high-pass at low crossover)
        const float y_hp = hp1.b0 * x + hp1.b1 * hp1.x1 + hp1.b2 * hp1.x2
                          - hp1.a1 * hp1.y1 - hp1.a2 * hp1.y2;
        hp1.x2 = hp1.x1; hp1.x1 = x;
        hp1.y2 = hp1.y1; hp1.y1 = y_hp;
        hp1Out[i] = y_hp;
    }

    // Second stage: split hp1Out at highCrossover
    for (int i = 0; i < numSamples; ++i)
    {
        const float x = hp1Out[i];

        // LP2 (low-pass at high crossover) = mid band
        const float y_lp = lp2.b0 * x + lp2.b1 * lp2.x1 + lp2.b2 * lp2.x2
                          - lp2.a1 * lp2.y1 - lp2.a2 * lp2.y2;
        lp2.x2 = lp2.x1; lp2.x1 = x;
        lp2.y2 = lp2.y1; lp2.y1 = y_lp;
        midData[i] = y_lp;

        // HP2 (high-pass at high crossover) = high band
        const float y_hp = hp2.b0 * x + hp2.b1 * hp2.x1 + hp2.b2 * hp2.x2
                          - hp2.a1 * hp2.y1 - hp2.a2 * hp2.y2;
        hp2.x2 = hp2.x1; hp2.x1 = x;
        hp2.y2 = hp2.y1; hp2.y1 = y_hp;
        highData[i] = y_hp;
    }
}

void MultibandOtt::processBand(int bandIndex, float* data, int numSamples)
{
    if (bandIndex < 0 || bandIndex >= 3)
        return;

    auto& state = bandStates[bandIndex];

    float thresholdDb = 0.0f;
    float ratio = 4.0f;
    switch (bandIndex)
    {
        case 0: thresholdDb = lowThresholdDb; ratio = lowRatio; break;
        case 1: thresholdDb = midThresholdDb; ratio = midRatio; break;
        case 2: thresholdDb = highThresholdDb; ratio = highRatio; break;
    }

    const float thresholdLinear = std::pow(10.0f, thresholdDb / 20.0f);
    const float invRatio = 1.0f - 1.0f / ratio;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inputLevel = std::abs(data[i]);

        // Envelope follower
        if (inputLevel > state.envelope)
            state.envelope = state.attackCoeff * state.envelope + (1.0f - state.attackCoeff) * inputLevel;
        else
            state.envelope = state.releaseCoeff * state.envelope + (1.0f - state.releaseCoeff) * inputLevel;

        // Downward compression
        float gain = 1.0f;
        if (state.envelope > thresholdLinear && state.envelope > 0.0f)
        {
            const float dbOver = 20.0f * std::log10(state.envelope / thresholdLinear);
            const float dbCompressed = dbOver * invRatio;
            gain = std::pow(10.0f, -dbCompressed / 20.0f);
        }

        // Upward compression (OTT-style: boost signals below threshold)
        if (upwardAmount > 0.0f && state.envelope < thresholdLinear && state.envelope > 0.0f)
        {
            const float dbUnder = 20.0f * std::log10(thresholdLinear / state.envelope);
            const float upwardGain = 1.0f + (std::pow(10.0f, dbUnder / 20.0f) - 1.0f) * upwardAmount * 0.3f;
            gain *= upwardGain;
        }

        data[i] *= gain;
    }
}

void MultibandOtt::mergeBands(float* output, int numSamples)
{
    const float* lowData = bandBuffer0.getReadPointer(0);
    const float* midData = bandBuffer1.getReadPointer(0);
    const float* highData = bandBuffer2.getReadPointer(0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = lowData[i] + midData[i] + highData[i];
}

void MultibandOtt::setActive(bool active) { this->active = active; }
bool MultibandOtt::isActive() const { return active; }
void MultibandOtt::setLowThresholdDb(float db) { lowThresholdDb = std::clamp(db, -60.0f, 0.0f); }
void MultibandOtt::setMidThresholdDb(float db) { midThresholdDb = std::clamp(db, -60.0f, 0.0f); }
void MultibandOtt::setHighThresholdDb(float db) { highThresholdDb = std::clamp(db, -60.0f, 0.0f); }
void MultibandOtt::setLowRatio(float ratio) { lowRatio = std::clamp(ratio, 1.0f, 20.0f); }
void MultibandOtt::setMidRatio(float ratio) { midRatio = std::clamp(ratio, 1.0f, 20.0f); }
void MultibandOtt::setHighRatio(float ratio) { highRatio = std::clamp(ratio, 1.0f, 20.0f); }
void MultibandOtt::setMakeupDb(float db) { makeupDb = std::clamp(db, -12.0f, 12.0f); }
void MultibandOtt::setMix(float mix) { this->mix = std::clamp(mix, 0.0f, 1.0f); }
