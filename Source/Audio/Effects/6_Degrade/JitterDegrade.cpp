#include "JitterDegrade.h"

#include <algorithm>
#include <cmath>

void JitterDegrade::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 0.35));
    reset();
    updateDelaySamples();
}

void JitterDegrade::updateDelaySamples() {
    if (delayBuffer.getNumSamples() <= 2) {
        baseDelaySamples = 1;
        jitterRangeSamples = 1;
        return;
    }

    const int maxDelay = delayBuffer.getNumSamples() - 2;
    baseDelaySamples = std::clamp(
        static_cast<int>(std::round(baseDelayMs * 0.001f * static_cast<float>(sampleRate))),
        1,
        maxDelay);
    jitterRangeSamples = std::clamp(
        static_cast<int>(std::round(jitterMs * 0.001f * static_cast<float>(sampleRate))),
        1,
        maxDelay);
}

void JitterDegrade::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int capacity = delayBuffer.getNumSamples();
    if (capacity <= 2) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int sample = 0; sample < numSamples; ++sample) {
        rngState = rngState * 1664525u + 1013904223u;
        const float random01 = static_cast<float>((rngState >> 8) & 0xFFFFu) / 65535.0f;
        const float randomBipolar = random01 * 2.0f - 1.0f;
        const int jitterOffset = static_cast<int>(randomBipolar * static_cast<float>(jitterRangeSamples));

        int readPos = writePos - baseDelaySamples - jitterOffset;
        while (readPos < 0) {
            readPos += capacity;
        }
        readPos %= capacity;

        for (int ch = 0; ch < channels; ++ch) {
            const float input = buffer.getSample(ch, sample);
            const float delayed = delayBuffer.getSample(ch, readPos);
            delayBuffer.setSample(ch, writePos, input);
            const float output = input * (1.0f - mix) + delayed * mix;
            buffer.setSample(ch, sample, output);
        }

        writePos = (writePos + 1) % capacity;
    }
}

void JitterDegrade::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool JitterDegrade::isActive() const {
    return active;
}

void JitterDegrade::reset() {
    delayBuffer.clear();
    writePos = 0;
    rngState = 0x12345678u;
}

void JitterDegrade::setBaseDelayMs(float ms) {
    baseDelayMs = std::clamp(ms, 2.0f, 80.0f);
    updateDelaySamples();
}

void JitterDegrade::setJitterMs(float ms) {
    jitterMs = std::clamp(ms, 1.0f, 40.0f);
    updateDelaySamples();
}

void JitterDegrade::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
