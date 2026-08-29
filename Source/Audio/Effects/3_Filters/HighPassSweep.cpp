#include "HighPassSweep.h"
#include <algorithm>

HighPassSweep::HighPassSweep() {}

void HighPassSweep::prepare(double sampleRate, int blockSize) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 2;

    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    filter.setResonance(1.5f);
    reset();
}

void HighPassSweep::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active && currentCutoff <= 20.1f) return;

    float target = active ? targetCutoff : 20.0f;
    currentCutoff += (target - currentCutoff) * sweepRate;
    filter.setCutoffFrequency(currentCutoff);

    juce::dsp::AudioBlock<float> block(buffer);
    auto subBlock = block.getSubBlock(0, static_cast<size_t>(numSamples));
    juce::dsp::ProcessContextReplacing<float> context(subBlock);
    filter.process(context);
}

void HighPassSweep::setActive(bool beActive) {
    active = beActive;
}

bool HighPassSweep::isActive() const { return active; }

void HighPassSweep::setTargetCutoff(float hz) {
    targetCutoff = std::clamp(hz, 200.0f, 12000.0f);
}

void HighPassSweep::setSweepRate(float amount) {
    sweepRate = std::clamp(amount, 0.005f, 0.2f);
}

void HighPassSweep::reset() {
    filter.reset();
    currentCutoff = 20.0f;
    filter.setCutoffFrequency(currentCutoff);
}
