#include "DelayEcho.h"

#include <algorithm>
#include <cmath>

void DelayEcho::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = newSampleRate;
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 2.0));
    reset();
    updateDelaySamples();
}

void DelayEcho::updateDelaySamples() {
    if (delayBuffer.getNumSamples() <= 1) {
        delaySamples = 1;
        return;
    }

    const float samples = (delayMs * 0.001f) * static_cast<float>(sampleRate);
    delaySamples = std::clamp(static_cast<int>(std::round(samples)), 1, delayBuffer.getNumSamples() - 1);
}

void DelayEcho::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
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
            delayBuffer.setSample(ch, writePos, input + delayed * feedback);
            const float output = input * (1.0f - mix) + delayed * mix;
            buffer.setSample(ch, sample, output);
        }

        writePos = (writePos + 1) % capacity;
    }
}

void DelayEcho::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool DelayEcho::isActive() const {
    return active;
}

void DelayEcho::reset() {
    delayBuffer.clear();
    writePos = 0;
}

void DelayEcho::setDelayMs(float ms) {
    delayMs = std::clamp(ms, 40.0f, 1200.0f);
    updateDelaySamples();
}

void DelayEcho::setFeedback(float amount) {
    feedback = std::clamp(amount, 0.0f, 0.95f);
}

void DelayEcho::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
