#include "Phaser.h"

#include <algorithm>
#include <cmath>

void Phaser::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    reset();
}

void Phaser::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float phaseInc = twoPi * rateHz / static_cast<float>(sampleRate);

    for (int sample = 0; sample < numSamples; ++sample) {
        const float sweep = 0.5f + 0.5f * std::sin(phase);
        const float a = std::clamp(0.15f + depth * (0.78f * sweep), 0.05f, 0.95f);
        for (int ch = 0; ch < channels; ++ch) {
            const float dry = buffer.getSample(ch, sample);
            float y = dry;
            for (float& state : z1[ch]) {
                const float allpass = -a * y + state;
                state = y + a * allpass;
                y = allpass;
            }
            buffer.setSample(ch, sample, dry * (1.0f - mix) + y * mix);
        }
        phase += phaseInc;
        if (phase >= twoPi) {
            phase -= twoPi;
        }
    }
}

void Phaser::setActive(bool beActive) { active = beActive; }
bool Phaser::isActive() const { return active; }
void Phaser::reset() { for (auto& ch : z1) ch.fill(0.0f); phase = 0.0f; }
void Phaser::setRateHz(float hz) { rateHz = std::clamp(hz, 0.03f, 8.0f); }
void Phaser::setDepth(float amount) { depth = std::clamp(amount, 0.0f, 1.0f); }
void Phaser::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
