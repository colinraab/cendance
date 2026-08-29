#include "PeakLimiter.h"

#include <algorithm>
#include <cmath>

void PeakLimiter::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = newSampleRate;
    updateReleaseCoefficient();
    setCeilingDb(ceilingDb);
    reset();
}

void PeakLimiter::updateReleaseCoefficient() {
    const float releaseSeconds = std::max(0.001f, releaseMs * 0.001f);
    const float sr = static_cast<float>(std::max(1.0, sampleRate));
    releaseCoeff = std::exp(-1.0f / (releaseSeconds * sr));
}

void PeakLimiter::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int ch = 0; ch < channels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        float channelGain = gain[ch];

        for (int sample = 0; sample < numSamples; ++sample) {
            const float input = channelData[sample];
            const float absInput = std::abs(input) + 1.0e-12f;
            const float targetGain = absInput > ceilingLinear ? (ceilingLinear / absInput) : 1.0f;

            if (targetGain < channelGain) {
                channelGain = targetGain;
            } else {
                channelGain = releaseCoeff * channelGain + (1.0f - releaseCoeff) * targetGain;
            }

            channelData[sample] = input * channelGain;
        }

        gain[ch] = channelGain;
    }
}

void PeakLimiter::setActive(bool beActive) {
    active = beActive;
}

bool PeakLimiter::isActive() const {
    return active;
}

void PeakLimiter::reset() {
    gain[0] = 1.0f;
    gain[1] = 1.0f;
}

void PeakLimiter::setCeilingDb(float db) {
    ceilingDb = std::clamp(db, -12.0f, -0.1f);
    ceilingLinear = std::pow(10.0f, ceilingDb / 20.0f);
}

void PeakLimiter::setReleaseMs(float ms) {
    releaseMs = std::clamp(ms, 5.0f, 400.0f);
    updateReleaseCoefficient();
}
