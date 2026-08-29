#include "TapeStop.h"
#include <algorithm>

TapeStop::TapeStop() {}

void TapeStop::updateSpeedDelta() {
    const float clampedSeconds = std::max(0.05f, stopTimeSeconds);
    speedDelta = 1.0f / static_cast<float>(clampedSeconds * sampleRate);
}

void TapeStop::prepare(double newSampleRate, int blockSize) {
    sampleRate = newSampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 2; 

    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    updateSpeedDelta();
    reset();
}

void TapeStop::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) return;
    
    for (int s = 0; s < numSamples; ++s) {
        currentSpeed = std::max(0.0f, currentSpeed - speedDelta);
        delayTimeSamples += (1.0f - currentSpeed);
        
        if (delayTimeSamples >= static_cast<float>(sampleRate) * 1.9f) {
            delayTimeSamples = static_cast<float>(sampleRate) * 1.9f;
        }

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            float in = buffer.getSample(ch, s);
            delayLine.pushSample(ch, in);
            float out = delayLine.popSample(ch, delayTimeSamples);
            buffer.setSample(ch, s, out * currentSpeed);
        }
    }
}

void TapeStop::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool TapeStop::isActive() const {
    return active;
}

void TapeStop::setStopTimeSeconds(float seconds) {
    stopTimeSeconds = seconds;
    updateSpeedDelta();
}

void TapeStop::reset() {
    delayLine.reset();
    currentSpeed = 1.0f;
    delayTimeSamples = 0.0f;
}
