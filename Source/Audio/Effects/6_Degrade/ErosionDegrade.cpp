#include "ErosionDegrade.h"

#include <algorithm>
#include <cmath>

void ErosionDegrade::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    updateHoldSamples();
    reset();
}

void ErosionDegrade::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int sample = 0; sample < numSamples; ++sample) {
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);

            if (remainingHold[ch] <= 0) {
                rngState = rngState * 1664525u + 1013904223u;
                const float random01 = static_cast<float>((rngState >> 8) & 0xFFFFu) / 65535.0f;
                if (random01 < dropProbability) {
                    heldValues[ch] = 0.0f;
                    remainingHold[ch] = holdSamples;
                } else {
                    heldValues[ch] = dry;
                    remainingHold[ch] = 1;
                }
            }

            const float wet = heldValues[ch];
            buffer.setSample(ch, sample, dry * (1.0f - mix) + wet * mix);
            --remainingHold[ch];
        }
    }
}

void ErosionDegrade::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool ErosionDegrade::isActive() const {
    return active;
}

void ErosionDegrade::reset() {
    remainingHold = {{0, 0}};
    heldValues = {{0.0f, 0.0f}};
    rngState = 0x9E3779B9u;
}

void ErosionDegrade::setDropProbability(float amount) {
    dropProbability = std::clamp(amount, 0.0f, 1.0f);
}

void ErosionDegrade::setHoldMs(float ms) {
    holdMs = std::clamp(ms, 1.0f, 100.0f);
    updateHoldSamples();
}

void ErosionDegrade::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void ErosionDegrade::updateHoldSamples() {
    const float samples = holdMs * 0.001f * static_cast<float>(sampleRate);
    holdSamples = std::max(1, static_cast<int>(std::round(samples)));
}
