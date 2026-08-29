#include "SaturationWaveshaper.h"

#include <algorithm>
#include <cmath>

void SaturationWaveshaper::prepare(double sampleRate, int blockSize) {
    juce::ignoreUnused(sampleRate, blockSize);
    reset();
}

void SaturationWaveshaper::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float inputGain = std::pow(10.0f, inputGainDb / 20.0f);
    const float trim = std::pow(10.0f, outputTrimDb / 20.0f);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int sample = 0; sample < numSamples; ++sample) {
            const float dry = channelData[sample];
            const float shaped = std::tanh(((dry * inputGain) + bias) * drive);
            channelData[sample] = (dry * (1.0f - mix) + shaped * mix) * trim;
        }
    }
}

void SaturationWaveshaper::setActive(bool beActive) {
    active = beActive;
}

bool SaturationWaveshaper::isActive() const {
    return active;
}

void SaturationWaveshaper::reset() {
}

void SaturationWaveshaper::setDrive(float amount) {
    drive = std::clamp(amount, 1.0f, 12.0f);
}

void SaturationWaveshaper::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void SaturationWaveshaper::setInputGainDb(float db) {
    inputGainDb = std::clamp(db, -24.0f, 24.0f);
}

void SaturationWaveshaper::setOutputTrimDb(float db) {
    outputTrimDb = std::clamp(db, -24.0f, 12.0f);
}

void SaturationWaveshaper::setBias(float amount) {
    bias = std::clamp(amount, -0.25f, 0.25f);
}
