#include "SidechainDucker.h"

#include <algorithm>
#include <cmath>

void SidechainDucker::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    smoothingCoefficient = std::exp(-1.0f / (0.002f * static_cast<float>(sampleRate)));
    updatePeriodSamples();
    reset();
}

void SidechainDucker::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    for (int sample = 0; sample < numSamples; ++sample) {
        const float phase01 = static_cast<float>(phaseSamples) / static_cast<float>(std::max(1, periodSamples));
        const float eased = std::pow(1.0f - std::clamp(phase01, 0.0f, 1.0f), 1.0f + curve * 3.0f);
        const float targetGain = (1.0f - depth) + depth * (1.0f - eased);
        smoothedGain = targetGain + smoothingCoefficient * (smoothedGain - targetGain);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            buffer.setSample(ch, sample, buffer.getSample(ch, sample) * smoothedGain);
        }

        ++phaseSamples;
        if (phaseSamples >= periodSamples) {
            phaseSamples = 0;
        }
    }
}

void SidechainDucker::setActive(bool beActive) {
    active = beActive;
}

bool SidechainDucker::isActive() const {
    return active;
}

void SidechainDucker::reset() {
    phaseSamples = 0;
    smoothedGain = 1.0f;
}

void SidechainDucker::setBpm(float newBpm) {
    bpm = std::clamp(newBpm, 20.0f, 260.0f);
    updatePeriodSamples();
}

void SidechainDucker::setRepeatDivision(float division) {
    repeatDivision = std::clamp(division, 0.03125f, 1.0f);
    updatePeriodSamples();
}

void SidechainDucker::setDepth(float amount) {
    depth = std::clamp(amount, 0.0f, 1.0f);
}

void SidechainDucker::setCurve(float amount) {
    curve = std::clamp(amount, 0.0f, 1.0f);
}

void SidechainDucker::updatePeriodSamples() {
    const float beatSamples = (60.0f / std::max(1.0f, bpm)) * static_cast<float>(sampleRate);
    periodSamples = std::max(1, static_cast<int>(std::round(beatSamples * repeatDivision)));
}
