#include "PitchShifter.h"

#include <algorithm>
#include <cmath>

void PitchShifter::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 0.25));
    reset();
}

float PitchShifter::readDelay(int channel, float position) const {
    const int capacity = delayBuffer.getNumSamples();
    while (position < 0.0f) {
        position += static_cast<float>(capacity);
    }
    while (position >= static_cast<float>(capacity)) {
        position -= static_cast<float>(capacity);
    }
    const int i0 = static_cast<int>(position) % capacity;
    const int i1 = (i0 + 1) % capacity;
    const float frac = position - std::floor(position);
    return delayBuffer.getSample(channel, i0) * (1.0f - frac)
        + delayBuffer.getSample(channel, i1) * frac;
}

void PitchShifter::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active || delayBuffer.getNumSamples() <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    const int capacity = delayBuffer.getNumSamples();
    const float windowSamples = std::clamp(windowMs * 0.001f * static_cast<float>(sampleRate), 64.0f, capacity * 0.45f);
    const float ratio = std::pow(2.0f, semitones / 12.0f);
    const float phaseInc = std::clamp(ratio - 1.0f, -0.95f, 3.0f);

    for (int sample = 0; sample < numSamples; ++sample) {
        const float rampA = readPhase / windowSamples;
        const float rampB = std::fmod(readPhase + windowSamples * 0.5f, windowSamples) / windowSamples;
        const float gainA = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * rampA);
        const float gainB = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * rampB);

        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            delayBuffer.setSample(ch, writePos, dry);
            const float posA = static_cast<float>(writePos) - readPhase - 16.0f;
            const float posB = static_cast<float>(writePos) - std::fmod(readPhase + windowSamples * 0.5f, windowSamples) - 16.0f;
            const float wet = (readDelay(ch, posA) * gainA + readDelay(ch, posB) * gainB) / std::max(0.001f, gainA + gainB);
            buffer.setSample(ch, sample, dry * (1.0f - mix) + wet * mix);
        }

        readPhase += phaseInc;
        while (readPhase < 0.0f) {
            readPhase += windowSamples;
        }
        while (readPhase >= windowSamples) {
            readPhase -= windowSamples;
        }
        writePos = (writePos + 1) % capacity;
    }
}

void PitchShifter::setActive(bool beActive) { active = beActive; }
bool PitchShifter::isActive() const { return active; }
void PitchShifter::reset() { delayBuffer.clear(); writePos = 0; readPhase = 0.0f; }
void PitchShifter::setSemitones(float value) { semitones = std::clamp(value, -24.0f, 24.0f); }
void PitchShifter::setWindowMs(float ms) { windowMs = std::clamp(ms, 25.0f, 180.0f); }
void PitchShifter::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
