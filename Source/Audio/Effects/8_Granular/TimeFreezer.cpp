#include "TimeFreezer.h"

#include <algorithm>

void TimeFreezer::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = std::max(1.0, newSampleRate);
    const int maxSamples = std::max(1, static_cast<int>(sampleRate * 4.0));
    freezeBuffer.setSize(2, maxSamples);
    reset();
    updateFreezeWindowSamples();
}

void TimeFreezer::updateFreezeWindowSamples() {
    if (freezeBuffer.getNumSamples() <= 1) {
        freezeWindowSamples = 1;
        return;
    }

    const float samples = freezeWindowMs * 0.001f * static_cast<float>(sampleRate);
    freezeWindowSamples = std::clamp(static_cast<int>(samples), 1, freezeBuffer.getNumSamples() - 1);
    if (hasSnapshot) {
        hasSnapshot = false;
        writePos = 0;
        readPos = 0.0f;
    }
}

void TimeFreezer::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int capacity = freezeBuffer.getNumSamples();
    if (capacity <= 1 || freezeWindowSamples <= 1) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    if (channels <= 0) {
        return;
    }

    for (int sample = 0; sample < numSamples; ++sample) {
        if (!hasSnapshot) {
            for (int ch = 0; ch < channels; ++ch) {
                freezeBuffer.setSample(ch, writePos, buffer.getSample(ch, sample));
            }

            ++writePos;
            if (writePos >= freezeWindowSamples) {
                hasSnapshot = true;
                writePos = 0;
                readPos = 0.0f;
            }
            continue;
        }

        const int baseIndex = static_cast<int>(readPos);
        const int nextIndex = (baseIndex + 1) % freezeWindowSamples;
        const float frac = readPos - static_cast<float>(baseIndex);

        for (int ch = 0; ch < channels; ++ch) {
            const float frozenA = freezeBuffer.getSample(ch, baseIndex);
            const float frozenB = freezeBuffer.getSample(ch, nextIndex);
            const float frozen = frozenA + (frozenB - frozenA) * frac;
            const float dry = buffer.getSample(ch, sample);
            buffer.setSample(ch, sample, dry * (1.0f - mix) + frozen * mix);
        }

        readPos += playbackRate;
        while (readPos >= static_cast<float>(freezeWindowSamples)) {
            readPos -= static_cast<float>(freezeWindowSamples);
        }
    }
}

void TimeFreezer::setActive(bool beActive) {
    if (active == beActive) {
        return;
    }

    if (beActive && freezeBuffer.getNumSamples() <= 0) {
        active = false;
        return;
    }

    active = beActive;
    reset();
}

bool TimeFreezer::isActive() const {
    return active;
}

void TimeFreezer::reset() {
    freezeBuffer.clear();
    hasSnapshot = false;
    writePos = 0;
    readPos = 0.0f;
}

void TimeFreezer::setFreezeWindowMs(float ms) {
    freezeWindowMs = std::clamp(ms, 40.0f, 3000.0f);
    updateFreezeWindowSamples();
}

void TimeFreezer::setPlaybackRate(float rate) {
    playbackRate = std::clamp(rate, 0.25f, 2.0f);
}

void TimeFreezer::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}
