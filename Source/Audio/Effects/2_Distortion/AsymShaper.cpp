#include "AsymShaper.h"

#include <algorithm>
#include <cmath>

void AsymShaper::prepare(double sampleRate, int blockSize) {
    juce::ignoreUnused(sampleRate, blockSize);
    reset();
}

void AsymShaper::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float positiveScale = 1.0f + asymmetry;
    const float negativeScale = 1.0f - asymmetry;
    const float inputGain = std::pow(10.0f, inputGainDb / 20.0f);
    const float outputGain = std::pow(10.0f, outputGainDb / 20.0f);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int sample = 0; sample < numSamples; ++sample) {
            const float dry = channelData[sample];
            const float driven = dry * inputGain * drive;
            const float shaped = driven >= 0.0f
                ? std::tanh(driven * positiveScale)
                : std::tanh(driven * negativeScale);
            channelData[sample] = (dry * (1.0f - mix) + shaped * mix) * outputGain;
        }
    }
}

void AsymShaper::setActive(bool beActive) {
    active = beActive;
}

bool AsymShaper::isActive() const {
    return active;
}

void AsymShaper::reset() {
}

void AsymShaper::setDrive(float amount) {
    drive = std::clamp(amount, 1.0f, 12.0f);
}

void AsymShaper::setAsymmetry(float amount) {
    asymmetry = std::clamp(amount, 0.0f, 0.95f);
}

void AsymShaper::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void AsymShaper::setInputGainDb(float db) {
    inputGainDb = std::clamp(db, -24.0f, 24.0f);
}

void AsymShaper::setOutputGainDb(float db) {
    outputGainDb = std::clamp(db, -24.0f, 12.0f);
}
