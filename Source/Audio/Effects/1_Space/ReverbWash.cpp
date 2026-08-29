#include "ReverbWash.h"
#include <algorithm>

ReverbWash::ReverbWash() {}

void ReverbWash::applyParams() {
    juce::dsp::Reverb::Parameters params;
    params.roomSize = roomSize;
    params.damping = damping;
    params.wetLevel = 1.0f;
    params.dryLevel = 0.0f;
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    reverb.setParameters(params);
}

void ReverbWash::prepare(double sampleRate, int blockSize) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 2;

    reverb.prepare(spec);
    
    applyParams();
    
    wetBuffer.setSize(2, blockSize);
    reset();
}

void ReverbWash::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active && sendAmount <= 0.001f) return;
    
    float target = active ? targetMix : 0.0f;
    
    if (wetBuffer.getNumSamples() < numSamples) {
        wetBuffer.setSize(buffer.getNumChannels(), numSamples, true);
    }
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        if (ch < wetBuffer.getNumChannels()) {
            wetBuffer.copyFrom(ch, 0, buffer.getReadPointer(ch), numSamples);
        }
    }
    
    juce::dsp::AudioBlock<float> block(wetBuffer);
    auto subBlock = block.getSubBlock(0, static_cast<size_t>(numSamples));
    juce::dsp::ProcessContextReplacing<float> context(subBlock);
    reverb.process(context);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        if (ch >= wetBuffer.getNumChannels()) continue;

        float* dry = buffer.getWritePointer(ch);
        const float* wet = wetBuffer.getReadPointer(ch);
        
        float amount = sendAmount;
        for (int s = 0; s < numSamples; ++s) {
            amount += (target - amount) * 0.001f;
            dry[s] = dry[s] * (1.0f - amount) + wet[s] * amount;
        }
        
        if (ch == buffer.getNumChannels() - 1) {
            sendAmount = amount;
        }
    }
}

void ReverbWash::setActive(bool beActive) {
    active = beActive;
}

bool ReverbWash::isActive() const { return active; }

void ReverbWash::setTargetMix(float mixAmount) {
    targetMix = std::clamp(mixAmount, 0.0f, 1.0f);
}

void ReverbWash::setRoomSize(float size) {
    roomSize = std::clamp(size, 0.0f, 1.0f);
    applyParams();
}

void ReverbWash::setDamping(float dampingAmount) {
    damping = std::clamp(dampingAmount, 0.0f, 1.0f);
    applyParams();
}

void ReverbWash::reset() {
    reverb.reset();
    sendAmount = 0.0f;
}
