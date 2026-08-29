#include "CompressorGlue.h"

#include <algorithm>
#include <cmath>

void CompressorGlue::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = newSampleRate;
    updateCoefficients();
    reset();
}

void CompressorGlue::updateCoefficients() {
    const float attackSeconds = std::max(0.001f, attackMs * 0.001f);
    const float releaseSeconds = std::max(0.001f, releaseMs * 0.001f);
    const float sr = static_cast<float>(std::max(1.0, sampleRate));
    attackCoeff = std::exp(-1.0f / (attackSeconds * sr));
    releaseCoeff = std::exp(-1.0f / (releaseSeconds * sr));
}

void CompressorGlue::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float makeupGain = std::pow(10.0f, makeupDb / 20.0f);
    const int channels = std::min(2, buffer.getNumChannels());

    for (int ch = 0; ch < channels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        float env = envelope[ch];

        for (int sample = 0; sample < numSamples; ++sample) {
            const float input = channelData[sample];
            const float detector = std::abs(input) + 1.0e-9f;

            if (detector > env) {
                env = attackCoeff * env + (1.0f - attackCoeff) * detector;
            } else {
                env = releaseCoeff * env + (1.0f - releaseCoeff) * detector;
            }

            const float envDb = 20.0f * std::log10(env);
            const float overDb = envDb - thresholdDb;
            const float reductionDb = overDb > 0.0f ? (overDb - (overDb / ratio)) : 0.0f;
            const float gain = std::pow(10.0f, (-reductionDb) / 20.0f) * makeupGain;
            channelData[sample] = input * gain;
        }

        envelope[ch] = env;
    }
}

void CompressorGlue::setActive(bool beActive) {
    active = beActive;
}

bool CompressorGlue::isActive() const {
    return active;
}

void CompressorGlue::reset() {
    envelope[0] = 0.0f;
    envelope[1] = 0.0f;
}

void CompressorGlue::setThresholdDb(float db) {
    thresholdDb = std::clamp(db, -48.0f, 0.0f);
}

void CompressorGlue::setRatio(float newRatio) {
    ratio = std::clamp(newRatio, 1.0f, 20.0f);
}

void CompressorGlue::setMakeupDb(float db) {
    makeupDb = std::clamp(db, -12.0f, 18.0f);
}
