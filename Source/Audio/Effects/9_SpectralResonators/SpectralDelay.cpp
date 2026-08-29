#include "SpectralDelay.h"

#include <algorithm>
#include <cmath>

void SpectralDelay::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;

    inputBuffer.setSize(1, fftSize * 2, false, false, true);
    outputBuffer.setSize(1, fftSize * 2, false, false, true);
    inputBuffer.clear();
    outputBuffer.clear();

    writePos = 0;

    for (auto& ch : channelStates)
    {
        for (auto& c : ch.fftBuffer) c = {};
        ch.windowedFrame.fill(0.0f);
        ch.overlapBuffer.fill(0.0f);
        ch.prevPhase.fill(0.0f);
        ch.accumPhase.fill(0.0f);
        ch.delayRingBuffer.setSize(1, static_cast<int>(sampleRate) * maxDelaySeconds, false, false, true);
        ch.delayRingBuffer.clear();
        ch.delayWritePos = 0;
        ch.delaySamples = static_cast<int>(delayMs * sampleRate / 1000.0);
    }

    lpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * crossoverFreq / static_cast<float>(sampleRate));
    lpCoeff = std::clamp(lpCoeff, 0.001f, 1.0f);

    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
}

void SpectralDelay::reset()
{
    inputBuffer.clear();
    outputBuffer.clear();
    writePos = 0;

    for (auto& ch : channelStates)
    {
        for (auto& c : ch.fftBuffer) c = {};
        ch.windowedFrame.fill(0.0f);
        ch.overlapBuffer.fill(0.0f);
        ch.prevPhase.fill(0.0f);
        ch.accumPhase.fill(0.0f);
        ch.delayRingBuffer.clear();
        ch.delayWritePos = 0;
    }
}

void SpectralDelay::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1)
        return;

    for (int ch = 0; ch < numChannels && ch < delayChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        processFFTChannel(channelData, numSamples, ch);
    }
}

void SpectralDelay::processFFTChannel(float* channelData, int numSamples, int channel)
{
    auto& state = channelStates[channel];
    const int delayLen = state.delayRingBuffer.getNumSamples();

    // Pre-compute Hann window
    auto hannWindow = [](int i, int N) -> float {
        return 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(N)));
    };

    int inputPos = 0;
    for (int i = 0; i < numSamples; ++i)
    {
        // Write input to delay ring buffer
        state.delayRingBuffer.setSample(0, state.delayWritePos, channelData[i]);

        // Read from delay
        const int readPos = (state.delayWritePos - state.delaySamples + delayLen) % delayLen;
        const float delayed = state.delayRingBuffer.getSample(0, readPos);

        // Accumulate into FFT frame
        state.windowedFrame[inputPos] = delayed * hannWindow(inputPos, fftSize);
        ++inputPos;

        if (inputPos >= fftSize)
        {
            // Forward FFT using member FFT
            fft->performRealOnlyForwardTransform(state.windowedFrame.data());

            // Process each frequency bin with phase shifts
            for (int k = 0; k < fftSize / 2; ++k)
            {
                const float re = state.windowedFrame[k * 2];
                const float im = state.windowedFrame[k * 2 + 1];
                const float mag = std::sqrt(re * re + im * im);
                const float phase = std::atan2(im, re);

                const float normalizedFreq = static_cast<float>(k) / static_cast<float>(fftSize / 2);
                const float delayScale = 1.0f - normalizedFreq * lpCoeff;
                const float phaseShift = delayScale * juce::MathConstants<float>::twoPi * 0.1f;

                state.accumPhase[k] += phaseShift;
                const float newPhase = phase + state.accumPhase[k];

                state.windowedFrame[k * 2] = mag * std::cos(newPhase);
                state.windowedFrame[k * 2 + 1] = mag * std::sin(newPhase);
            }

            // Inverse FFT
            fft->performRealOnlyInverseTransform(state.windowedFrame.data());

            // Overlap-add
            for (int n = 0; n < fftSize; ++n)
            {
                state.overlapBuffer[n] += state.windowedFrame[n] * hannWindow(n, fftSize);
            }

            // Shift for next frame
            memmove(state.windowedFrame.data(), state.windowedFrame.data() + hopSize,
                    (fftSize - hopSize) * sizeof(float));
            memset(state.windowedFrame.data() + fftSize - hopSize, 0, hopSize * sizeof(float));

            inputPos = fftSize - hopSize;
        }

        // Output: mix dry + processed delayed signal
        const float processedDelayed = state.overlapBuffer[0];
        memmove(state.overlapBuffer.data(), state.overlapBuffer.data() + 1, (fftSize - 1) * sizeof(float));
        state.overlapBuffer[fftSize - 1] = 0.0f;

        channelData[i] = channelData[i] * (1.0f - mix) + processedDelayed * mix;

        // Write feedback
        state.delayRingBuffer.setSample(0, state.delayWritePos, channelData[i] + delayed * feedback);

        state.delayWritePos = (state.delayWritePos + 1) % delayLen;
    }
}

void SpectralDelay::setActive(bool active) { this->active = active; }
bool SpectralDelay::isActive() const { return active; }
void SpectralDelay::setDelayMs(float ms) { delayMs = std::clamp(ms, 1.0f, 2000.0f); }
void SpectralDelay::setFeedback(float amount) { feedback = std::clamp(amount, 0.0f, 0.95f); }
void SpectralDelay::setCrossoverFreq(float freq) { crossoverFreq = std::clamp(freq, 100.0f, 10000.0f); }
void SpectralDelay::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
