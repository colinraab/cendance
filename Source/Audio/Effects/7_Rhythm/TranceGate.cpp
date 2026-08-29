#include "TranceGate.h"

#include <algorithm>
#include <cmath>

void TranceGate::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    smoothingCoefficient = std::exp(-1.0f / (0.002f * static_cast<float>(sampleRate)));
    updatePeriodSamples();
    reset();
}

void TranceGate::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float closedGain = 1.0f - depth;
    const int openSamples = std::max(1, static_cast<int>(std::round(static_cast<float>(periodSamples) * dutyCycle)));

    for (int sample = 0; sample < numSamples; ++sample) {
        const int phase = phaseSamples % periodSamples;
        const float targetGain = phase < openSamples ? 1.0f : closedGain;
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

void TranceGate::setActive(bool beActive) {
    active = beActive;
}

bool TranceGate::isActive() const {
    return active;
}

void TranceGate::reset() {
    phaseSamples = 0;
    smoothedGain = 1.0f;
}

void TranceGate::setBpm(float newBpm) {
    bpm = std::clamp(newBpm, 20.0f, 260.0f);
    updatePeriodSamples();
}

void TranceGate::setRepeatDivision(float division) {
    repeatDivision = std::clamp(division, 0.03125f, 1.0f);
    updatePeriodSamples();
}

void TranceGate::setDepth(float amount) {
    depth = std::clamp(amount, 0.0f, 1.0f);
}

void TranceGate::setDutyCycle(float amount) {
    dutyCycle = std::clamp(amount, 0.05f, 0.95f);
}

void TranceGate::updatePeriodSamples() {
    const float beatSamples = (60.0f / std::max(1.0f, bpm)) * static_cast<float>(sampleRate);
    periodSamples = std::max(1, static_cast<int>(std::round(beatSamples * repeatDivision)));
}
