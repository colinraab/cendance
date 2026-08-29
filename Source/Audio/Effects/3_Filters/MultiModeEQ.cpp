#include "MultiModeEQ.h"

#include <algorithm>

void MultiModeEQ::prepare(double newSampleRate, int blockSize) {
    sampleRate = std::max(1.0, newSampleRate);
    juce::ignoreUnused(blockSize);

    for (auto& filter : filters) {
        filter.reset();
    }

    reset();
    updateCoefficients();
}

void MultiModeEQ::updateCoefficients() {
    if (sampleRate <= 1.0) {
        return;
    }

    juce::IIRCoefficients coefficients;

    switch (mode) {
        case Mode::HighPass:
            coefficients = juce::IIRCoefficients::makeHighPass(sampleRate,
                                                               frequencyHz,
                                                               resonanceQ);
            break;
        case Mode::LowPass:
            coefficients = juce::IIRCoefficients::makeLowPass(sampleRate,
                                                              frequencyHz,
                                                              resonanceQ);
            break;
        case Mode::BandPass:
            coefficients = juce::IIRCoefficients::makeBandPass(sampleRate,
                                                               frequencyHz,
                                                               resonanceQ);
            break;
        case Mode::Notch:
            coefficients = juce::IIRCoefficients::makeNotchFilter(sampleRate,
                                      frequencyHz,
                                      resonanceQ);
            break;
        case Mode::Bell:
            coefficients = juce::IIRCoefficients::makePeakFilter(sampleRate,
                                     frequencyHz,
                                     resonanceQ,
                                     juce::Decibels::decibelsToGain(bellGainDb));
            break;
    }

    for (auto& filter : filters) {
        filter.setCoefficients(coefficients);
    }
}

void MultiModeEQ::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    if (!active) {
        return;
    }

    const int channels = std::min(2, buffer.getNumChannels());
    for (int ch = 0; ch < channels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        auto& filter = filters[static_cast<size_t>(ch)];
        for (int sample = 0; sample < numSamples; ++sample) {
            channelData[sample] = filter.processSingleSampleRaw(channelData[sample]);
        }
    }
}

void MultiModeEQ::setActive(bool beActive) {
    if (active != beActive) {
        active = beActive;
        if (!active) {
            reset();
        }
    }
}

bool MultiModeEQ::isActive() const {
    return active;
}

void MultiModeEQ::reset() {
    for (auto& filter : filters) {
        filter.reset();
    }
}

void MultiModeEQ::setFrequencyHz(float hz) {
    frequencyHz = std::clamp(hz, 20.0f, 18000.0f);
    updateCoefficients();
}

void MultiModeEQ::setShape(float value) {
    resonanceQ = std::clamp(value, 0.25f, 12.0f);
    bellGainDb = std::clamp(value, -18.0f, 18.0f);
    updateCoefficients();
}

void MultiModeEQ::setModeSelector(float value) {
    const float selector = std::clamp(value, 0.0f, 1.0f);
    if (selector < 0.2f) {
        mode = Mode::HighPass;
    } else if (selector < 0.4f) {
        mode = Mode::LowPass;
    } else if (selector < 0.6f) {
        mode = Mode::BandPass;
    } else if (selector < 0.8f) {
        mode = Mode::Notch;
    } else {
        mode = Mode::Bell;
    }

    updateCoefficients();
}
