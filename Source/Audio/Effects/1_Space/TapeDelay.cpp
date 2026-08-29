#include "TapeDelay.h"

#include <algorithm>
#include <cmath>

void TapeDelay::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;

    // Allocate delay buffer for max 2 seconds
    const int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    delayBuffer.setSize(2, maxDelaySamples, false, false, true);
    delayBuffer.clear();

    writePos = 0;
    wowPhase = 0.0f;
    hfState[0] = 0.0f;
    hfState[1] = 0.0f;

    updateDelaySamples();
}

void TapeDelay::reset()
{
    delayBuffer.clear();
    writePos = 0;
    wowPhase = 0.0f;
    hfState[0] = 0.0f;
    hfState[1] = 0.0f;
}

void TapeDelay::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1)
        return;

    const int maxDelaySamples = delayBuffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            // LFO for wow/flutter
            wowPhase += wowRate / static_cast<float>(sampleRate);
            if (wowPhase >= 1.0f)
                wowPhase -= 1.0f;
            const float lfo = std::sin(wowPhase * juce::MathConstants<float>::twoPi) * wowDepth;

            // Modulated delay read position
            const float modulatedDelay = static_cast<float>(delaySamples) * (1.0f + lfo);
            const int modDelay = std::max(1, static_cast<int>(modulatedDelay));
            const int readPos = (writePos - modDelay + maxDelaySamples) % maxDelaySamples;

            // Linear interpolation for fractional delay
            const float frac = modulatedDelay - static_cast<float>(modDelay);
            const int readPos2 = (readPos + 1) % maxDelaySamples;
            const float delayed = delayBuffer.getSample(ch, readPos) * (1.0f - frac)
                                + delayBuffer.getSample(ch, readPos2) * frac;

            // HF rolloff (simple 1-pole LP on feedback path)
            const float hfFiltered = hfState[ch] + hfCoeff * (delayed - hfState[ch]);
            hfState[ch] = hfFiltered;

            // Saturation on feedback
            const float saturated = saturate(hfFiltered * feedback);

            // Write to delay buffer
            delayBuffer.setSample(ch, writePos, data[i] + saturated);

            // Mix output
            data[i] = data[i] * (1.0f - mix) + delayed * mix;

            writePos = (writePos + 1) % maxDelaySamples;
        }
    }
}

float TapeDelay::saturate(float x)
{
    // Soft clipping: tanh-like saturation
    const float drive = saturation;
    const float driven = x * drive;
    return driven / (1.0f + std::abs(driven));
}

void TapeDelay::setActive(bool active) { this->active = active; }
bool TapeDelay::isActive() const { return active; }
void TapeDelay::setDelayMs(float ms) { delayMs = std::clamp(ms, 1.0f, 2000.0f); updateDelaySamples(); }
void TapeDelay::setFeedback(float amount) { feedback = std::clamp(amount, 0.0f, 0.95f); }
void TapeDelay::setSaturation(float amount) { saturation = std::clamp(amount, 0.0f, 10.0f); }
void TapeDelay::setWowRate(float rateHz) { wowRate = std::clamp(rateHz, 0.0f, 10.0f); }
void TapeDelay::setWowDepth(float depth) { wowDepth = std::clamp(depth, 0.0f, 1.0f); }
void TapeDelay::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }

void TapeDelay::updateDelaySamples()
{
    delaySamples = static_cast<int>(delayMs * sampleRate / 1000.0);
    delaySamples = std::max(1, delaySamples);

    // HF rolloff coefficient (higher = more rolloff)
    hfCoeff = 0.15f;
}
