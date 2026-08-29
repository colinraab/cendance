#include "Flanger.h"

#include <algorithm>
#include <cmath>

void Flanger::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 0.03));
    reset();
}

float Flanger::readDelay(int channel, float delaySamples) const {
    const int capacity = delayBuffer.getNumSamples();
    float read = static_cast<float>(writePos) - delaySamples;
    while (read < 0.0f) {
        read += static_cast<float>(capacity);
    }
    const int i0 = static_cast<int>(read) % capacity;
    const int i1 = (i0 + 1) % capacity;
    const float frac = read - std::floor(read);
    return delayBuffer.getSample(channel, i0) * (1.0f - frac)
        + delayBuffer.getSample(channel, i1) * frac;
}

void Flanger::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active || delayBuffer.getNumSamples() <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float phaseInc = twoPi * rateHz / static_cast<float>(sampleRate);
    const float sr = static_cast<float>(sampleRate);

    for (int sample = 0; sample < numSamples; ++sample) {
        const float delaySamples = (1.0f + 5.0f * (0.5f + 0.5f * std::sin(phase))) * 0.001f * sr;
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            const float delayed = readDelay(ch, delaySamples);
            delayBuffer.setSample(ch, writePos, dry + delayed * feedback);
            buffer.setSample(ch, sample, dry * (1.0f - mix) + delayed * mix);
        }
        phase += phaseInc;
        if (phase >= twoPi) {
            phase -= twoPi;
        }
        writePos = (writePos + 1) % delayBuffer.getNumSamples();
    }
}

void Flanger::setActive(bool beActive) { active = beActive; }
bool Flanger::isActive() const { return active; }
void Flanger::reset() { delayBuffer.clear(); writePos = 0; phase = 0.0f; }
void Flanger::setRateHz(float hz) { rateHz = std::clamp(hz, 0.03f, 6.0f); }
void Flanger::setFeedback(float amount) { feedback = std::clamp(amount, -0.90f, 0.90f); }
void Flanger::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
