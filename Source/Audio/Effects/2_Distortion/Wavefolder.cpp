#include "Wavefolder.h"

#include <algorithm>
#include <cmath>

void Wavefolder::prepare(double sampleRate, int blockSize) {
    juce::ignoreUnused(sampleRate, blockSize);
    reset();
}

void Wavefolder::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const float threshold = 1.0f - (0.8f * foldAmount);
    const float thresholdSafe = std::max(0.05f, threshold);
    const float inputGain = std::pow(10.0f, inputGainDb / 20.0f);
    const float outputGain = std::pow(10.0f, outputGainDb / 20.0f);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int sample = 0; sample < numSamples; ++sample) {
            const float dry = channelData[sample];
            float folded = dry * inputGain * drive;

            for (int guard = 0; guard < 6; ++guard) {
                if (folded > thresholdSafe) {
                    folded = (2.0f * thresholdSafe) - folded;
                } else if (folded < -thresholdSafe) {
                    folded = (-2.0f * thresholdSafe) - folded;
                } else {
                    break;
                }
            }

            const float normalized = std::clamp(folded / thresholdSafe, -1.0f, 1.0f);
            channelData[sample] = (dry * (1.0f - mix) + normalized * mix) * outputGain;
        }
    }
}

void Wavefolder::setActive(bool beActive) {
    active = beActive;
}

bool Wavefolder::isActive() const {
    return active;
}

void Wavefolder::reset() {
}

void Wavefolder::setDrive(float amount) {
    drive = std::clamp(amount, 1.0f, 10.0f);
}

void Wavefolder::setFoldAmount(float amount) {
    foldAmount = std::clamp(amount, 0.0f, 1.0f);
}

void Wavefolder::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void Wavefolder::setInputGainDb(float db) {
    inputGainDb = std::clamp(db, -24.0f, 24.0f);
}

void Wavefolder::setOutputGainDb(float db) {
    outputGainDb = std::clamp(db, -24.0f, 12.0f);
}
