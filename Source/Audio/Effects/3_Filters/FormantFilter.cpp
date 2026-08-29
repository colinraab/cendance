#include "FormantFilter.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr std::array<std::array<float, 3>, 4> kVowels{{
    {{730.0f, 1090.0f, 2440.0f}},
    {{530.0f, 1840.0f, 2480.0f}},
    {{270.0f, 2290.0f, 3010.0f}},
    {{570.0f, 840.0f, 2410.0f}},
}};
}

void FormantFilter::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    reset();
}

float FormantFilter::processBand(float input, int channel, int band, float frequency) {
    auto& state = states[channel][band];
    const float f = 2.0f * std::sin(juce::MathConstants<float>::pi
        * std::clamp(frequency, 80.0f, 8000.0f) / static_cast<float>(sampleRate));
    const float q = 0.55f + resonance * 1.35f;
    state.lp += f * state.bp;
    const float hp = input - state.lp - q * state.bp;
    state.bp += f * hp;
    return state.bp;
}

void FormantFilter::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    const float scaled = std::clamp(vowel, 0.0f, 1.0f) * 3.0f;
    const int index = std::min(2, static_cast<int>(scaled));
    const float frac = scaled - static_cast<float>(index);

    for (int sample = 0; sample < numSamples; ++sample) {
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            float wet = 0.0f;
            for (int band = 0; band < 3; ++band) {
                const float freq = kVowels[index][band] * (1.0f - frac) + kVowels[index + 1][band] * frac;
                wet += processBand(dry, ch, band, freq);
            }
            wet *= 1.8f;
            buffer.setSample(ch, sample, dry * (1.0f - mix) + wet * mix);
        }
    }
}

void FormantFilter::setActive(bool beActive) { active = beActive; }
bool FormantFilter::isActive() const { return active; }
void FormantFilter::reset() { for (auto& ch : states) for (auto& band : ch) band = {}; }
void FormantFilter::setVowel(float selector) { vowel = std::clamp(selector, 0.0f, 1.0f); }
void FormantFilter::setResonance(float amount) { resonance = std::clamp(amount, 0.0f, 1.0f); }
void FormantFilter::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
