#include "CloudGenerator.h"

#include <algorithm>
#include <cmath>

void CloudGenerator::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;

    // Delay buffer for capturing input (2 seconds)
    delayBuffer.setSize(2, static_cast<int>(sampleRate * 2.0), false, false, true);
    delayBuffer.clear();

    // Cloud output buffer
    cloudBuffer.setSize(2, newBlockSize, false, false, true);
    cloudBuffer.clear();

    writePos = 0;
    samplesUntilNextGrain = 0;
    randomState = 0x12345678u;

    for (auto& grain : grains)
        grain = {};
}

void CloudGenerator::reset()
{
    delayBuffer.clear();
    cloudBuffer.clear();
    writePos = 0;
    samplesUntilNextGrain = 0;

    for (auto& grain : grains)
        grain = {};
}

void CloudGenerator::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1)
        return;

    const int delayLen = delayBuffer.getNumSamples();

    // Write input to delay buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* src = buffer.getReadPointer(ch);
        float* dst = delayBuffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            dst[(writePos + i) % delayLen] = src[i];
    }

    // Clear cloud buffer
    cloudBuffer.clear();

    // Calculate grain spawn rate
    const int grainLengthSamples = static_cast<int>(grainSizeMs * sampleRate / 1000.0f);
    const int spraySamples = static_cast<int>(spray * sampleRate / 1000.0f);
    const float samplesPerGrain = density > 0.1f ? sampleRate / density : sampleRate;

    // Process grains
    for (auto& grain : grains)
    {
        if (!grain.active)
            continue;

        for (int i = 0; i < numSamples; ++i)
        {
            const int grainPos = grain.startSample + i;
            if (grainPos >= grain.lengthSamples)
            {
                grain.active = false;
                break;
            }

            // Hann window envelope
            const float windowPhase = static_cast<float>(grainPos) / static_cast<float>(grain.lengthSamples);
            const float envelope = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * windowPhase));

            // Read from delay buffer with pitch shift
            const float readPosF = position * static_cast<float>(delayLen) + grain.positionSamples
                                 + static_cast<float>(grainPos) * grain.pitchRatio;
            const int readPos = (static_cast<int>(readPosF) % delayLen + delayLen) % delayLen;

            // Linear interpolation
            const float frac = readPosF - static_cast<float>(static_cast<int>(readPosF));
            const int readPos2 = (readPos + 1) % delayLen;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* delayData = delayBuffer.getReadPointer(ch);
                const float sample = delayData[readPos] * (1.0f - frac) + delayData[readPos2] * frac;
                cloudBuffer.addSample(ch, i, sample * envelope * grain.amplitude);
            }
        }

        grain.startSample += numSamples;
    }

    // Spawn new grains
    for (int i = 0; i < numSamples; ++i)
    {
        --samplesUntilNextGrain;
        if (samplesUntilNextGrain <= 0)
        {
            // Find inactive grain
            for (auto& grain : grains)
            {
                if (grain.active)
                    continue;

                grain.active = true;
                grain.startSample = 0;
                grain.lengthSamples = grainLengthSamples;
                grain.positionSamples = static_cast<int>((nextRandom() / 4294967295.0f) * spraySamples);
                grain.pitchRatio = std::pow(2.0f, (nextRandom() / 4294967295.0f * 2.0f - 1.0f) * pitchScatter / 12.0f);
                grain.amplitude = 0.3f / std::sqrt(density);

                // Random pan
                const float pan = nextRandom() / 4294967295.0f;
                grain.panL = std::cos(pan * juce::MathConstants<float>::halfPi);
                grain.panR = std::sin(pan * juce::MathConstants<float>::halfPi);

                break;
            }

            samplesUntilNextGrain = static_cast<int>(samplesPerGrain);
            if (samplesUntilNextGrain < 1)
                samplesUntilNextGrain = 1;
        }
    }

    // Mix cloud with dry signal
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        const float* cloud = cloudBuffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = data[i] * (1.0f - mix) + cloud[i] * mix;
    }

    writePos = (writePos + numSamples) % delayLen;
}

uint32_t CloudGenerator::nextRandom()
{
    // xorshift32
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return randomState;
}

void CloudGenerator::setActive(bool active) { this->active = active; }
bool CloudGenerator::isActive() const { return active; }
void CloudGenerator::setGrainSizeMs(float ms) { grainSizeMs = std::clamp(ms, 1.0f, 2000.0f); }
void CloudGenerator::setDensity(float grainsPerSecond) { density = std::clamp(grainsPerSecond, 0.1f, 100.0f); }
void CloudGenerator::setPitchScatter(float semitones) { pitchScatter = std::clamp(semitones, 0.0f, 24.0f); }
void CloudGenerator::setPosition(float pos) { position = std::clamp(pos, 0.0f, 1.0f); }
void CloudGenerator::setSpray(float ms) { spray = std::clamp(ms, 0.0f, 500.0f); }
void CloudGenerator::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
