#pragma once

#include "../MasterEffect.h"

#include <array>
#include <complex>
#include <juce_dsp/juce_dsp.h>
#include <memory>

class SpectralDelay : public MasterEffect {
public:
    SpectralDelay() = default;

    void prepare(double sampleRate, int blockSize) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void setActive(bool active) override;
    bool isActive() const override;
    void reset() override;

    void setDelayMs(float ms);
    void setFeedback(float amount);
    void setCrossoverFreq(float freq);
    void setMix(float amount);

private:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;
    static constexpr int maxDelaySeconds = 4;
    static constexpr int delayChannels = 2;

    void processFFTChannel(float* channelData, int numSamples, int channel);

    bool active = false;
    double sampleRate = 44100.0;
    float delayMs = 300.0f;
    float feedback = 0.35f;
    float crossoverFreq = 2000.0f;
    float mix = 0.35f;

    int writePos = 0;
    float lpCoeff = 0.0f;

    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;

    struct ChannelState {
        std::array<std::complex<float>, fftSize> fftBuffer;
        std::array<float, fftSize> windowedFrame;
        std::array<float, fftSize> overlapBuffer;
        std::array<float, fftSize> prevPhase;
        std::array<float, fftSize> accumPhase;
        juce::AudioBuffer<float> delayRingBuffer;
        int delayWritePos = 0;
        int delaySamples = 1;
    };

    std::array<ChannelState, delayChannels> channelStates;

    std::unique_ptr<juce::dsp::FFT> fft;
};
