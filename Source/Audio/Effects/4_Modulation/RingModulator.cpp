#include "RingModulator.h"

#include <algorithm>
#include <cmath>

void RingModulator::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    reset();
}

void RingModulator::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float twoPi = juce::MathConstants<float>::twoPi;
    const float phaseInc = twoPi * rateHz / static_cast<float>(sampleRate);

    for (int sample = 0; sample < numSamples; ++sample) {
        const float mod = std::sin(phase);
        const float gain = (1.0f - depth) + depth * mod;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            const float dry = buffer.getSample(ch, sample);
            const float ring = dry * gain;
            buffer.setSample(ch, sample, dry * (1.0f - mix) + ring * mix);
        }

        phase += phaseInc;
        if (phase >= twoPi) {
            phase -= twoPi;
        }
    }
}

void RingModulator::setActive(bool beActive) {
    active = beActive;
}

bool RingModulator::isActive() const {
    return active;
}

void RingModulator::reset() {
    phase = 0.0f;
}

void RingModulator::setRateHz(float hz) {
    rateHz = std::clamp(hz, 0.2f, 1000.0f);
}

void RingModulator::setDepth(float amount) {
    depth = std::clamp(amount, 0.0f, 1.0f);
}

void RingModulator::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
