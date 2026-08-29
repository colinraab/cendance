#include "PingPongDelay.h"

#include <algorithm>
#include <cmath>

void PingPongDelay::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;

    const int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    delayBufferL.setSize(1, maxDelaySamples, false, false, true);
    delayBufferR.setSize(1, maxDelaySamples, false, false, true);
    delayBufferL.clear();
    delayBufferR.clear();

    writePosL = 0;
    writePosR = 0;
    lpStateL = 0.0f;
    lpStateR = 0.0f;

    updateDelaySamples();
    updateCrossover();
}

void PingPongDelay::reset()
{
    delayBufferL.clear();
    delayBufferR.clear();
    writePosL = 0;
    writePosR = 0;
    lpStateL = 0.0f;
    lpStateR = 0.0f;
}

void PingPongDelay::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 2)
    {
        // Mono: just do regular delay
        if (numChannels == 1)
        {
            float* data = buffer.getWritePointer(0);
            const int maxLen = delayBufferL.getNumSamples();
            for (int i = 0; i < numSamples; ++i)
            {
                const int readPos = (writePosL - delaySamples + maxLen) % maxLen;
                const float delayed = delayBufferL.getSample(0, readPos);
                delayBufferL.setSample(0, writePosL, data[i] + delayed * feedback);
                data[i] = data[i] * (1.0f - mix) + delayed * mix;
                writePosL = (writePosL + 1) % maxLen;
            }
        }
        return;
    }

    // Stereo ping-pong
    float* leftData = buffer.getWritePointer(0);
    float* rightData = buffer.getWritePointer(1);
    const int maxLen = delayBufferL.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // Read from delay buffers
        const int readPosL = (writePosL - delaySamples + maxLen) % maxLen;
        const int readPosR = (writePosR - delaySamples + maxLen) % maxLen;

        const float delayedL = delayBufferL.getSample(0, readPosL);
        const float delayedR = delayBufferR.getSample(0, readPosR);

        // Crossover filtering: low frequencies stay, high frequencies ping-pong
        const float monoSum = (leftData[i] + rightData[i]) * 0.5f;
        const float monoDiff = (leftData[i] - rightData[i]) * 0.5f;

        // LP filter the mono component
        lpStateL += lpCoeff * (monoSum - lpStateL);
        lpStateR += lpCoeff * (monoSum - lpStateR);

        // Write to delay buffers: L delay gets R feedback, R delay gets L feedback (ping-pong)
        delayBufferL.setSample(0, writePosL, lpStateL + delayedR * feedback);
        delayBufferR.setSample(0, writePosR, lpStateR + delayedL * feedback);

        // Output: dry + wet
        const float outL = leftData[i] * (1.0f - mix) + delayedL * mix;
        const float outR = rightData[i] * (1.0f - mix) + delayedR * mix;

        leftData[i] = outL;
        rightData[i] = outR;

        writePosL = (writePosL + 1) % maxLen;
        writePosR = (writePosR + 1) % maxLen;
    }
}

void PingPongDelay::setActive(bool active) { this->active = active; }
bool PingPongDelay::isActive() const { return active; }
void PingPongDelay::setDelayMs(float ms) { delayMs = std::clamp(ms, 1.0f, 2000.0f); updateDelaySamples(); }
void PingPongDelay::setFeedback(float amount) { feedback = std::clamp(amount, 0.0f, 0.95f); }
void PingPongDelay::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
void PingPongDelay::setCrossoverFreq(float freq) { crossoverFreq = std::clamp(freq, 100.0f, 10000.0f); updateCrossover(); }

void PingPongDelay::updateDelaySamples()
{
    delaySamples = static_cast<int>(delayMs * sampleRate / 1000.0);
    delaySamples = std::max(1, delaySamples);
}

void PingPongDelay::updateCrossover()
{
    // Simple 1-pole LP coefficient
    lpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * crossoverFreq / static_cast<float>(sampleRate));
    lpCoeff = std::clamp(lpCoeff, 0.001f, 1.0f);
}
