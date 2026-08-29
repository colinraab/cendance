#include "BeatRepeatInsert.h"

#include <algorithm>

void BeatRepeatInsert::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = newSampleRate;
    crossfadeSamples = std::max(1, static_cast<int>(sampleRate * 0.002));
    transitionSamples = std::max(1, static_cast<int>(sampleRate * 0.005));
    wetGainStep = 1.0f / static_cast<float>(transitionSamples);
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 4.0));
    reset();
}

void BeatRepeatInsert::setBpm(float newBpm) {
    bpm = newBpm;
}

void BeatRepeatInsert::setRepeatDivision(float division) {
    repeatDivision = std::clamp(division, 0.03125f, 1.0f);
}

void BeatRepeatInsert::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void BeatRepeatInsert::setFeedback(float amount) {
    feedback = std::clamp(amount, 0.0f, 0.95f);
}

void BeatRepeatInsert::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    const int maxCapacity = delayBuffer.getNumSamples();
    if (maxCapacity == 0) {
        return;
    }

    for (int s = 0; s < numSamples; ++s) {
        if (!active && wetGain <= 0.0f) {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                if (ch < delayBuffer.getNumChannels()) {
                    delayBuffer.setSample(ch, writePos, buffer.getSample(ch, s));
                }
            }
            writePos = (writePos + 1) % maxCapacity;
            continue;
        }

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            if (ch >= delayBuffer.getNumChannels()) {
                continue;
            }

            const float dry = buffer.getSample(ch, s);
            float repeated = delayBuffer.getSample(ch, readPos);
            const int fadeLength = std::min(crossfadeSamples, std::max(1, repeatLength / 2));
            if (repeatPhase >= repeatLength - fadeLength) {
                const float fade = static_cast<float>(repeatPhase - (repeatLength - fadeLength))
                    / static_cast<float>(fadeLength);
                const int fadeInPos = (stutterStartIndex + repeatPhase - (repeatLength - fadeLength)) % maxCapacity;
                repeated += (delayBuffer.getSample(ch, fadeInPos) - repeated) * fade;
            }
            const float toWrite = dry + repeated * feedback;
            delayBuffer.setSample(ch, writePos, toWrite);
            const float effectiveMix = mix * wetGain;
            buffer.setSample(ch, s, repeated * effectiveMix + dry * (1.0f - effectiveMix));
        }

        readPos++;
        if (readPos >= stutterEndIndex) {
            readPos = stutterStartIndex;
        }

        writePos = (writePos + 1) % maxCapacity;
        repeatPhase = (repeatPhase + 1) % repeatLength;
        wetGain = std::clamp(wetGain + (targetActive ? wetGainStep : -wetGainStep), 0.0f, 1.0f);
        if (!targetActive && wetGain <= 0.0f) {
            active = false;
        }
    }
}

void BeatRepeatInsert::setActive(bool beActive) {
    if (targetActive != beActive) {
        const int bufferSamples = delayBuffer.getNumSamples();
        if (beActive && bufferSamples <= 0) {
            active = false;
            return;
        }

        targetActive = beActive;
        if (targetActive) {
            active = true;
            const float safeBpm = std::max(1.0f, bpm);
            const float beatDuration = (60.0f / safeBpm) * static_cast<float>(sampleRate);
            repeatLength = static_cast<int>(beatDuration * repeatDivision);
            if (repeatLength < 10) {
                repeatLength = 10;
            }
            repeatLength = std::min(repeatLength, bufferSamples);

            stutterEndIndex = writePos;
            stutterStartIndex = stutterEndIndex - repeatLength;
            stutterStartIndex %= bufferSamples;
            if (stutterStartIndex < 0) {
                stutterStartIndex += bufferSamples;
            }
            readPos = stutterStartIndex;
            repeatPhase = 0;
        }
    }
}

bool BeatRepeatInsert::isActive() const {
    return active;
}

void BeatRepeatInsert::reset() {
    delayBuffer.clear();
    writePos = 0;
    readPos = 0;
    stutterStartIndex = 0;
    stutterEndIndex = 0;
    repeatLength = 1;
    repeatPhase = 0;
    wetGain = 0.0f;
    targetActive = false;
}
