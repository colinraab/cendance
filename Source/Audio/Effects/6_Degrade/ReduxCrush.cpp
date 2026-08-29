#include "ReduxCrush.h"

#include <algorithm>
#include <cmath>

void ReduxCrush::prepare(double sampleRate, int blockSize) {
    juce::ignoreUnused(sampleRate, blockSize);
    reset();
}

float ReduxCrush::quantize(float input) const {
    const float clamped = std::clamp(input, -1.0f, 1.0f);
    const float levels = std::max(2.0f, std::pow(2.0f, bitDepth) - 1.0f);
    const float normalized = (clamped * 0.5f + 0.5f) * levels;
    const float quantized = std::round(normalized) / levels;
    return quantized * 2.0f - 1.0f;
}

void ReduxCrush::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int sample = 0; sample < numSamples; ++sample) {
        if (downsampleCounter <= 0) {
            for (int ch = 0; ch < channels; ++ch) {
                heldSample[ch] = quantize(buffer.getSample(ch, sample));
            }
            downsampleCounter = downsampleFactor;
        }

        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            const float wet = heldSample[ch];
            buffer.setSample(ch, sample, dry * (1.0f - mix) + wet * mix);
        }

        --downsampleCounter;
    }
}

void ReduxCrush::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool ReduxCrush::isActive() const {
    return active;
}

void ReduxCrush::reset() {
    downsampleCounter = 0;
    heldSample[0] = 0.0f;
    heldSample[1] = 0.0f;
}

void ReduxCrush::setBitDepth(float bits) {
    bitDepth = std::clamp(bits, 2.0f, 16.0f);
}

void ReduxCrush::setDownsampleFactor(float factor) {
    downsampleFactor = std::clamp(static_cast<int>(std::round(factor)), 1, 32);
}

void ReduxCrush::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
