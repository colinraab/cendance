#include "SoftHardClip.h"

#include <algorithm>
#include <cmath>

void SoftHardClip::prepare(double sampleRate, int blockSize) {
    juce::ignoreUnused(sampleRate, blockSize);
    reset();
}

void SoftHardClip::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float inputGain = std::pow(10.0f, inputGainDb / 20.0f);
    const float outputGain = std::pow(10.0f, outputGainDb / 20.0f);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int sample = 0; sample < numSamples; ++sample) {
            const float dry = channelData[sample];
            const float driven = dry * inputGain * drive;
            const float soft = std::tanh(driven);
            const float hard = std::clamp(driven, -1.0f, 1.0f);
            const float shaped = soft + (hard - soft) * hardness;
            channelData[sample] = (dry * (1.0f - mix) + shaped * mix) * outputGain;
        }
    }
}

void SoftHardClip::setActive(bool beActive) {
    active = beActive;
}

bool SoftHardClip::isActive() const {
    return active;
}

void SoftHardClip::reset() {
}

void SoftHardClip::setDrive(float amount) {
    drive = std::clamp(amount, 1.0f, 12.0f);
}

void SoftHardClip::setHardness(float amount) {
    hardness = std::clamp(amount, 0.0f, 1.0f);
}

void SoftHardClip::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void SoftHardClip::setInputGainDb(float db) {
    inputGainDb = std::clamp(db, -24.0f, 24.0f);
}

void SoftHardClip::setOutputGainDb(float db) {
    outputGainDb = std::clamp(db, -24.0f, 12.0f);
}
