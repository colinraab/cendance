#include "FrequencyShifter.h"

#include <algorithm>
#include <cmath>

void FrequencyShifter::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    reset();
}

float FrequencyShifter::processAllPass(float input, float coeff, AllPassState& state) {
    const float output = coeff * (input - state.y1) + state.x1;
    state.x1 = input;
    state.y1 = output;
    return output;
}

float FrequencyShifter::processChain(float input,
                                     const std::array<float, HilbertStageCount>& coeffs,
                                     HilbertStates& states) {
    float sample = input;
    for (int stage = 0; stage < HilbertStageCount; ++stage) {
        sample = processAllPass(sample, coeffs[static_cast<size_t>(stage)], states[static_cast<size_t>(stage)]);
    }
    return sample;
}

void FrequencyShifter::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active || buffer.getNumChannels() == 0) {
        return;
    }

    const float twoPi = juce::MathConstants<float>::twoPi;
    const float halfPi = juce::MathConstants<float>::halfPi;
    const float phaseInc = twoPi * shiftHz / static_cast<float>(sampleRate);
    const float rightPhaseOffset = halfPi * stereoPhase;
    const bool stereo = buffer.getNumChannels() > 1;

    for (int sample = 0; sample < numSamples; ++sample) {
        const float leftCos = std::cos(phase);
        const float leftSin = std::sin(phase);
        const float rightPhase = phase + rightPhaseOffset;
        const float rightCos = std::cos(rightPhase);
        const float rightSin = std::sin(rightPhase);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            const size_t stateChannel = static_cast<size_t>(std::min(ch, 1));
            const float dry = buffer.getSample(ch, sample);

            const float i = processChain(dry, kHilbertICoeffs, iStates[stateChannel]);
            const float q = processChain(dry, kHilbertQCoeffs, qStates[stateChannel]);

            const float modCos = (stereo && ch == 1) ? rightCos : leftCos;
            const float modSin = (stereo && ch == 1) ? rightSin : leftSin;
            const float shifted = i * modCos - q * modSin;
            buffer.setSample(ch, sample, dry * (1.0f - mix) + shifted * mix);
        }

        phase += phaseInc;
        while (phase >= twoPi) {
            phase -= twoPi;
        }
        while (phase < 0.0f) {
            phase += twoPi;
        }
    }
}

void FrequencyShifter::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool FrequencyShifter::isActive() const {
    return active;
}

void FrequencyShifter::reset() {
    phase = 0.0f;
    for (auto& channelStates : iStates) {
        for (auto& stage : channelStates) {
            stage = AllPassState{};
        }
    }
    for (auto& channelStates : qStates) {
        for (auto& stage : channelStates) {
            stage = AllPassState{};
        }
    }
}

void FrequencyShifter::setShiftHz(float hz) {
    shiftHz = std::clamp(hz, -1800.0f, 1800.0f);
}

void FrequencyShifter::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void FrequencyShifter::setStereoPhase(float amount) {
    stereoPhase = std::clamp(amount, 0.0f, 1.0f);
}
