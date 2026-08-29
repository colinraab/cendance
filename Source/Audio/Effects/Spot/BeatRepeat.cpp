#include "BeatRepeat.h"
#include <algorithm>

BeatRepeat::BeatRepeat() {}

void BeatRepeat::prepare(double newSampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate = newSampleRate;
    crossfadeSamples = std::max(1, static_cast<int>(sampleRate * 0.002));
    transitionSamples = std::max(1, static_cast<int>(sampleRate * 0.005));
    wetGainStep = 1.0f / static_cast<float>(transitionSamples);
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 4.0));
    reset();
}

void BeatRepeat::setBpm(float newBpm) {
    bpm = newBpm;
}

void BeatRepeat::setRepeatDivision(float division) {
    repeatDivision = std::clamp(division, 0.03125f, 1.0f);
}

void BeatRepeat::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

void BeatRepeat::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    int maxCapacity = delayBuffer.getNumSamples();
    if (maxCapacity == 0) return;

    for (int s = 0; s < numSamples; ++s) {
        if (!active && wetGain <= 0.0f) {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                if (ch < delayBuffer.getNumChannels()) {
                    delayBuffer.setSample(ch, writePos, buffer.getSample(ch, s));
                }
            }
            writePos = (writePos + 1) % maxCapacity;
        } else {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                if (ch < delayBuffer.getNumChannels()) {
                    const float dry = buffer.getSample(ch, s);
                    float out = delayBuffer.getSample(ch, readPos);
                    const int fadeLength = std::min(crossfadeSamples, std::max(1, repeatLength / 2));
                    if (repeatPhase >= repeatLength - fadeLength) {
                        const float fade = static_cast<float>(repeatPhase - (repeatLength - fadeLength))
                            / static_cast<float>(fadeLength);
                        const int fadeInPos = (stutterStartIndex + repeatPhase - (repeatLength - fadeLength)) % maxCapacity;
                        out += (delayBuffer.getSample(ch, fadeInPos) - out) * fade;
                    }
                    const float effectiveMix = mix * wetGain;
                    buffer.setSample(ch, s, out * effectiveMix + dry * (1.0f - effectiveMix));
                }
            }
            readPos++;
            if (readPos >= maxCapacity) {
                readPos = 0;
            }
            if (readPos == stutterEndIndex) {
                readPos = stutterStartIndex;
            }
            repeatPhase = (repeatPhase + 1) % repeatLength;

            wetGain = std::clamp(wetGain + (targetActive ? wetGainStep : -wetGainStep), 0.0f, 1.0f);
            if (!targetActive && wetGain <= 0.0f) {
                active = false;
            }
        }
    }
}

void BeatRepeat::setActive(bool beActive) {
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
            float beatDuration = (60.0f / safeBpm) * static_cast<float>(sampleRate);
            repeatLength = static_cast<int>(beatDuration * repeatDivision);
            if (repeatLength < 10) repeatLength = 10;
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

bool BeatRepeat::isActive() const { return active; }

void BeatRepeat::reset() {
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
