#include "Autopan.h"

#include <algorithm>
#include <cmath>

void Autopan::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    reset();
}

void Autopan::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active || buffer.getNumChannels() == 0) {
        return;
    }

    const float twoPi = juce::MathConstants<float>::twoPi;
    const float phaseInc = twoPi * rateHz / static_cast<float>(sampleRate);
    const bool stereo = buffer.getNumChannels() > 1;
    const float rightPhaseOffset = juce::MathConstants<float>::pi * width;

    for (int sample = 0; sample < numSamples; ++sample) {
        const float leftLfo = std::sin(phase);
        const float rightLfo = stereo ? std::sin(phase + rightPhaseOffset) : leftLfo;

        const float leftGain = (1.0f - depth) + depth * ((leftLfo + 1.0f) * 0.5f);
        const float rightGain = (1.0f - depth) + depth * ((rightLfo + 1.0f) * 0.5f);

        buffer.setSample(0, sample, buffer.getSample(0, sample) * leftGain);
        if (stereo) {
            buffer.setSample(1, sample, buffer.getSample(1, sample) * rightGain);
            for (int ch = 2; ch < buffer.getNumChannels(); ++ch) {
                buffer.setSample(ch, sample, buffer.getSample(ch, sample) * leftGain);
            }
        }

        phase += phaseInc;
        if (phase >= twoPi) {
            phase -= twoPi;
        }
    }
}

void Autopan::setActive(bool beActive) {
    active = beActive;
}

bool Autopan::isActive() const {
    return active;
}

void Autopan::reset() {
    phase = 0.0f;
}

void Autopan::setRateHz(float hz) {
    rateHz = std::clamp(hz, 0.05f, 12.0f);
}

void Autopan::setDepth(float amount) {
    depth = std::clamp(amount, 0.0f, 1.0f);
}

void Autopan::setWidth(float amount) {
    width = std::clamp(amount, 0.0f, 1.0f);
}
