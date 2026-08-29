#include "Harmonizer.h"

#include <algorithm>

void Harmonizer::prepare(double sampleRate, int blockSize) {
    voiceA.prepare(sampleRate, blockSize);
    voiceB.prepare(sampleRate, blockSize);
    workBuffer.setSize(2, std::max(1, blockSize));
    reset();
}

void Harmonizer::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    workBuffer.setSize(buffer.getNumChannels(), numSamples, false, false, true);
    workBuffer.makeCopyOf(buffer, true);
    voiceA.processBlock(buffer, numSamples);
    voiceB.processBlock(workBuffer, numSamples);

    const int channels = std::min(2, buffer.getNumChannels());
    for (int ch = 0; ch < channels; ++ch) {
        auto* a = buffer.getWritePointer(ch);
        const auto* b = workBuffer.getReadPointer(ch);
        for (int sample = 0; sample < numSamples; ++sample) {
            const float shifted = a[sample] * (1.0f - blend) + b[sample] * blend;
            a[sample] = a[sample] * (1.0f - mix) + shifted * mix;
        }
    }
}

void Harmonizer::setActive(bool beActive) {
    active = beActive;
    voiceA.setActive(beActive);
    voiceB.setActive(beActive);
}

bool Harmonizer::isActive() const { return active; }
void Harmonizer::reset() { voiceA.reset(); voiceB.reset(); workBuffer.clear(); }
void Harmonizer::setIntervalSemitones(float value) {
    interval = std::clamp(value, -12.0f, 12.0f);
    voiceA.setSemitones(interval);
    voiceB.setSemitones(interval >= 0.0f ? 12.0f : -12.0f);
}
void Harmonizer::setBlend(float amount) { blend = std::clamp(amount, 0.0f, 1.0f); }
void Harmonizer::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
    voiceA.setMix(1.0f);
    voiceB.setMix(1.0f);
}
