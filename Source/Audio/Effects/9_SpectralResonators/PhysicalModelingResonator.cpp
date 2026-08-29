#include "PhysicalModelingResonator.h"

#include <algorithm>
#include <cmath>

void PhysicalModelingResonator::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    const int maxDelaySamples = std::max(2, static_cast<int>(sampleRate * 0.25));
    delayBuffer.setSize(ResonatorCount * 2, maxDelaySamples);
    reset();
    updateDelaySamples();
}

void PhysicalModelingResonator::updateDelaySamples() {
    const int capacity = delayBuffer.getNumSamples();
    if (capacity <= 1) {
        delaySamples.fill(1);
        return;
    }

    const float maxHz = std::max(60.0f, static_cast<float>(sampleRate * 0.45));
    for (int i = 0; i < ResonatorCount; ++i) {
        const float targetHz = std::clamp(baseFrequencyHz * frequencyRatios[static_cast<size_t>(i)], 20.0f, maxHz);
        const float delay = static_cast<float>(sampleRate) / targetHz;
        delaySamples[static_cast<size_t>(i)] = std::clamp(static_cast<int>(std::round(delay)), 1, capacity - 1);
    }
}

void PhysicalModelingResonator::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int capacity = delayBuffer.getNumSamples();
    if (capacity <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    if (channels <= 0) {
        return;
    }

    const float feedback = resonance;
    const float dampingCoeff = std::clamp(0.08f + (1.0f - resonance) * 0.5f, 0.05f, 0.95f);

    for (int sample = 0; sample < numSamples; ++sample) {
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            float resonated = 0.0f;

            for (int resonator = 0; resonator < ResonatorCount; ++resonator) {
                const int readPos = (writePositions[static_cast<size_t>(resonator)]
                                     - delaySamples[static_cast<size_t>(resonator)]
                                     + capacity)
                    % capacity;
                const int stateChannel = resonator * 2 + ch;
                const float delayed = delayBuffer.getSample(stateChannel, readPos);

                const float previous = dampingState[static_cast<size_t>(resonator)][static_cast<size_t>(ch)];
                const float filtered = previous + dampingCoeff * (delayed - previous);
                dampingState[static_cast<size_t>(resonator)][static_cast<size_t>(ch)] = filtered;

                const float excitation = dry + filtered * feedback;
                delayBuffer.setSample(stateChannel,
                                      writePositions[static_cast<size_t>(resonator)],
                                      excitation);
                resonated += filtered;
            }

            resonated /= static_cast<float>(ResonatorCount);
            buffer.setSample(ch, sample, dry * (1.0f - mix) + resonated * mix);
        }

        for (int resonator = 0; resonator < ResonatorCount; ++resonator) {
            ++writePositions[static_cast<size_t>(resonator)];
            if (writePositions[static_cast<size_t>(resonator)] >= capacity) {
                writePositions[static_cast<size_t>(resonator)] = 0;
            }
        }
    }
}

void PhysicalModelingResonator::setActive(bool beActive) {
    if (active == beActive) {
        return;
    }

    if (beActive && delayBuffer.getNumSamples() <= 0) {
        active = false;
        return;
    }

    active = beActive;
    reset();
}

bool PhysicalModelingResonator::isActive() const {
    return active;
}

void PhysicalModelingResonator::reset() {
    delayBuffer.clear();
    writePositions.fill(0);
    for (auto& state : dampingState) {
        state.fill(0.0f);
    }
}

void PhysicalModelingResonator::setBaseFrequencyHz(float hz) {
    baseFrequencyHz = std::clamp(hz, 50.0f, 1600.0f);
    updateDelaySamples();
}

void PhysicalModelingResonator::setResonance(float amount) {
    resonance = std::clamp(amount, 0.0f, 0.98f);
}

void PhysicalModelingResonator::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
