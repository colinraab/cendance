#include "CombFilter.h"

#include <algorithm>
#include <cmath>

void CombFilter::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = newSampleRate;
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 0.5));
    reset();
    updateDelaySamples();
}

void CombFilter::updateDelaySamples() {
    if (delayBuffer.getNumSamples() <= 1) {
        delaySamples = 1;
        return;
    }

    const float samples = (delayMs * 0.001f) * static_cast<float>(sampleRate);
    delaySamples = std::clamp(static_cast<int>(std::round(samples)), 1, delayBuffer.getNumSamples() - 1);
}

void CombFilter::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int capacity = delayBuffer.getNumSamples();
    if (capacity <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int sample = 0; sample < numSamples; ++sample) {
        const int readPos = (writePos - delaySamples + capacity) % capacity;
        for (int ch = 0; ch < channels; ++ch) {
            const float input = buffer.getSample(ch, sample);
            const float delayed = delayBuffer.getSample(ch, readPos);
            const float comb = input + delayed * feedback;
            delayBuffer.setSample(ch, writePos, comb);
            const float output = input * (1.0f - mix) + delayed * mix;
            buffer.setSample(ch, sample, output);
        }

        writePos = (writePos + 1) % capacity;
    }
}

void CombFilter::setActive(bool beActive) {
    if (active != beActive) {
        if (beActive && delayBuffer.getNumSamples() <= 0) {
            active = false;
            return;
        }

        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool CombFilter::isActive() const {
    return active;
}

void CombFilter::reset() {
    delayBuffer.clear();
    writePos = 0;
}

void CombFilter::setDelayMs(float ms) {
    delayMs = std::clamp(ms, 2.0f, 120.0f);
    updateDelaySamples();
}

void CombFilter::setFeedback(float amount) {
    feedback = std::clamp(amount, -0.95f, 0.95f);
}

void CombFilter::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
