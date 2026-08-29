#pragma once

#include "../MasterEffect.h"

#include <array>
#include <complex>
#include <juce_dsp/juce_dsp.h>
#include <memory>

class SpectralBlur : public MasterEffect {
public:
    SpectralBlur() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setBlurTimeMs(float ms);
    void setFrequencyRange(float maxFreq);
    void setMix(float amount);

private:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;

    void processFFTChannel(float* channelData, int numSamples);

    bool active = false;
    double sampleRate = 44100.0;
    float blurTimeMs = 200.0f;
    float frequencyRange = 8000.0f;
    float mix = 0.50f;

    int writePos = 0;
    int inputPos = 0;
    float blurCoeff = 0.0f;
    float freqNorm = 1.0f;

    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;
    juce::AudioBuffer<float> windowBuffer;

    std::array<std::complex<float>, fftSize> fftBuffer;
    std::array<std::complex<float>, fftSize> prevFftBuffer;
    std::array<float, fftSize> magnitudeBuffer;
    std::array<float, fftSize> phaseBuffer;
    std::array<float, fftSize> windowedFrame;
    std::array<float, fftSize> overlapBuffer;

    std::unique_ptr<juce::dsp::FFT> fft;
};
