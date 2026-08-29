#include "SpectralBlur.h"

#include <juce_dsp/juce_dsp.h>

#include <algorithm>
#include <cmath>

void SpectralBlur::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;

    inputBuffer.setSize(1, fftSize * 2, false, false, true);
    outputBuffer.setSize(1, fftSize * 2, false, false, true);
    windowBuffer.setSize(1, fftSize, false, false, true);

    inputBuffer.clear();
    outputBuffer.clear();

    // Hann window
    float* win = windowBuffer.getWritePointer(0);
    for (int i = 0; i < fftSize; ++i)
        win[i] = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(fftSize)));

    writePos = 0;
    inputPos = 0;

    for (auto& c : fftBuffer) c = {};
    for (auto& c : prevFftBuffer) c = {};
    magnitudeBuffer.fill(0.0f);
    phaseBuffer.fill(0.0f);
    windowedFrame.fill(0.0f);
    overlapBuffer.fill(0.0f);

    blurCoeff = std::exp(-1000.0f / (blurTimeMs * static_cast<float>(sampleRate)));
    freqNorm = frequencyRange / static_cast<float>(sampleRate * 0.5);

    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
}

void SpectralBlur::reset()
{
    inputBuffer.clear();
    outputBuffer.clear();
    writePos = 0;
    inputPos = 0;

    for (auto& c : fftBuffer) c = {};
    for (auto& c : prevFftBuffer) c = {};
    magnitudeBuffer.fill(0.0f);
    phaseBuffer.fill(0.0f);
    windowedFrame.fill(0.0f);
    overlapBuffer.fill(0.0f);
}

void SpectralBlur::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1)
        return;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        processFFTChannel(channelData, numSamples);
    }
}

void SpectralBlur::processFFTChannel(float* channelData, int numSamples)
{
    const int outputLen = outputBuffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // Write input
        inputBuffer.setSample(0, inputPos, channelData[i]);

        // Read output
        const float outSample = outputBuffer.getSample(0, writePos) + overlapBuffer[inputPos % fftSize];
        channelData[i] = channelData[i] * (1.0f - mix) + outSample * mix;

        overlapBuffer[inputPos % fftSize] = 0.0f;

        ++inputPos;
        if (inputPos >= fftSize)
        {
            // Process frame
            float* inputData = inputBuffer.getWritePointer(0);
            memcpy(windowedFrame.data(), inputData, fftSize * sizeof(float));

            // Apply window
            const float* win = windowBuffer.getReadPointer(0);
            for (int j = 0; j < fftSize; ++j)
                windowedFrame[j] *= win[j];

            // Forward FFT
            fft->performRealOnlyForwardTransform(windowedFrame.data());

            // Convert to polar, apply blur, convert back
            const int maxBin = std::min(fftSize / 2, static_cast<int>(freqNorm * fftSize));
            for (int k = 0; k < maxBin; ++k)
            {
                const float re = windowedFrame[k * 2];
                const float im = windowedFrame[k * 2 + 1];
                const float mag = std::sqrt(re * re + im * im);
                const float phase = std::atan2(im, re);

                magnitudeBuffer[k] = blurCoeff * magnitudeBuffer[k] + (1.0f - blurCoeff) * mag;
                phaseBuffer[k] = phase;

                windowedFrame[k * 2] = magnitudeBuffer[k] * std::cos(phaseBuffer[k]);
                windowedFrame[k * 2 + 1] = magnitudeBuffer[k] * std::sin(phaseBuffer[k]);
            }

            // Zero bins above frequency range
            for (int k = maxBin; k < fftSize / 2; ++k)
            {
                windowedFrame[k * 2] = 0.0f;
                windowedFrame[k * 2 + 1] = 0.0f;
                magnitudeBuffer[k] = 0.0f;
            }

            // Inverse FFT
            fft->performRealOnlyInverseTransform(windowedFrame.data());

            // Overlap-add to output
            for (int n = 0; n < fftSize; ++n)
            {
                const int outIdx = (writePos + n) % outputLen;
                outputBuffer.addSample(0, outIdx, windowedFrame[n] * win[n]);
            }

            // Shift input buffer
            memmove(inputBuffer.getWritePointer(0), inputBuffer.getWritePointer(0) + hopSize,
                    (fftSize - hopSize) * sizeof(float));
            memset(inputBuffer.getWritePointer(0) + fftSize - hopSize, 0, hopSize * sizeof(float));

            inputPos = fftSize - hopSize;
        }

        writePos = (writePos + 1) % outputLen;
    }
}

void SpectralBlur::setActive(bool active) { this->active = active; }
bool SpectralBlur::isActive() const { return active; }
void SpectralBlur::setBlurTimeMs(float ms) { blurTimeMs = std::clamp(ms, 1.0f, 5000.0f); }
void SpectralBlur::setFrequencyRange(float maxFreq) { frequencyRange = std::clamp(maxFreq, 100.0f, 20000.0f); }
void SpectralBlur::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
