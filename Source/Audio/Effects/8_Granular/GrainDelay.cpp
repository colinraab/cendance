#include "GrainDelay.h"

#include <algorithm>
#include <cmath>

void GrainDelay::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    delayCrossfadeSamples = std::max(1, static_cast<int>(sampleRate * 0.002));
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 2.0));
    reset();
}

uint32_t GrainDelay::nextRandom() {
    randomState = randomState * 1664525u + 1013904223u;
    return randomState;
}

void GrainDelay::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active || delayBuffer.getNumSamples() <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    const int capacity = delayBuffer.getNumSamples();
    const int grainSamples = std::clamp(static_cast<int>(grainMs * 0.001f * static_cast<float>(sampleRate)), 32, capacity / 2);

    for (int sample = 0; sample < numSamples; ++sample) {
        if (--grainCounter <= 0) {
            const float r = static_cast<float>(nextRandom() & 0xffffu) / 65535.0f;
            previousDelay = currentDelay;
            currentDelay = std::clamp(static_cast<int>((grainMs * (0.5f + scatter * r)) * 0.001f * static_cast<float>(sampleRate)), 1, capacity - 1);
            delayCrossfadeRemaining = delayCrossfadeSamples;
            grainCounter = std::max(16, grainSamples / 2);
        }

        const int readPos = (writePos - currentDelay + capacity) % capacity;
        const int previousReadPos = (writePos - previousDelay + capacity) % capacity;
        const float delayFade = 1.0f - static_cast<float>(delayCrossfadeRemaining)
            / static_cast<float>(delayCrossfadeSamples);
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            const float currentWet = delayBuffer.getSample(ch, readPos);
            const float previousWet = delayBuffer.getSample(ch, previousReadPos);
            const float wet = previousWet + (currentWet - previousWet) * delayFade;
            delayBuffer.setSample(ch, writePos, dry + wet * 0.25f);
            buffer.setSample(ch, sample, dry * (1.0f - mix) + wet * mix);
        }
        delayCrossfadeRemaining = std::max(0, delayCrossfadeRemaining - 1);
        writePos = (writePos + 1) % capacity;
    }
}

void GrainDelay::setActive(bool beActive) { active = beActive; }
bool GrainDelay::isActive() const { return active; }
void GrainDelay::reset() { delayBuffer.clear(); writePos = 0; grainCounter = 0; currentDelay = 1; previousDelay = 1; delayCrossfadeRemaining = 0; }
void GrainDelay::setGrainMs(float ms) { grainMs = std::clamp(ms, 35.0f, 900.0f); }
void GrainDelay::setScatter(float amount) { scatter = std::clamp(amount, 0.0f, 1.0f); }
void GrainDelay::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
