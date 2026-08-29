#include "Chorus.h"

#include <algorithm>
#include <cmath>

void Chorus::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 0.08));
    reset();
}

float Chorus::readDelay(int channel, float delaySamples) const {
    const int capacity = delayBuffer.getNumSamples();
    if (capacity <= 1) {
        return 0.0f;
    }

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

void Chorus::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active || delayBuffer.getNumSamples() <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float phaseInc = twoPi * rateHz / static_cast<float>(sampleRate);
    const float baseDelay = 12.0f * 0.001f * static_cast<float>(sampleRate);
    const float depth = depthMs * 0.001f * static_cast<float>(sampleRate);

    for (int sample = 0; sample < numSamples; ++sample) {
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            delayBuffer.setSample(ch, writePos, dry);
            const float stereoPhase = ch == 0 ? 0.0f : juce::MathConstants<float>::pi;
            const float mod = 0.5f + 0.5f * std::sin(phase + stereoPhase);
            const float wet = readDelay(ch, baseDelay + depth * mod);
            buffer.setSample(ch, sample, dry * (1.0f - mix) + wet * mix);
        }

        phase += phaseInc;
        if (phase >= twoPi) {
            phase -= twoPi;
        }
        writePos = (writePos + 1) % delayBuffer.getNumSamples();
    }
}

void Chorus::setActive(bool beActive) { active = beActive; }
bool Chorus::isActive() const { return active; }
void Chorus::reset() { delayBuffer.clear(); writePos = 0; phase = 0.0f; }
void Chorus::setRateHz(float hz) { rateHz = std::clamp(hz, 0.05f, 8.0f); }
void Chorus::setDepthMs(float ms) { depthMs = std::clamp(ms, 1.0f, 35.0f); }
void Chorus::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
