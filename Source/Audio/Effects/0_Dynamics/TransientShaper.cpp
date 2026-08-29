#include "TransientShaper.h"

#include <algorithm>
#include <cmath>

void TransientShaper::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    updateCoefficients();
    reset();
}

void TransientShaper::updateCoefficients() {
    const float sr = static_cast<float>(std::max(1.0, sampleRate));
    fastCoeff = std::exp(-1.0f / (0.003f * sr));
    slowCoeff = std::exp(-1.0f / (0.080f * sr));
}

void TransientShaper::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int ch = 0; ch < channels; ++ch) {
        float fast = fastEnv[ch];
        float slow = slowEnv[ch];
        auto* data = buffer.getWritePointer(ch);

        for (int sample = 0; sample < numSamples; ++sample) {
            const float dry = data[sample];
            const float detector = std::abs(dry);
            fast = fastCoeff * fast + (1.0f - fastCoeff) * detector;
            slow = slowCoeff * slow + (1.0f - slowCoeff) * detector;
            const float transient = std::clamp((fast - slow) * 8.0f, -1.0f, 1.0f);
            const float body = std::clamp((slow - fast) * 5.0f, -1.0f, 1.0f);
            const float gain = std::clamp(1.0f + attack * transient + sustain * body, 0.0f, 3.0f);
            data[sample] = dry * (1.0f - mix) + dry * gain * mix;
        }

        fastEnv[ch] = fast;
        slowEnv[ch] = slow;
    }
}

void TransientShaper::setActive(bool beActive) { active = beActive; }
bool TransientShaper::isActive() const { return active; }
void TransientShaper::reset() { fastEnv.fill(0.0f); slowEnv.fill(0.0f); }
void TransientShaper::setAttack(float amount) { attack = std::clamp(amount, -1.0f, 1.0f); }
void TransientShaper::setSustain(float amount) { sustain = std::clamp(amount, -1.0f, 1.0f); }
void TransientShaper::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
