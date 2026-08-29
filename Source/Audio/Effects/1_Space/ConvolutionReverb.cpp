#include "ConvolutionReverb.h"

#include "IrBinaryData.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <cmath>

void ConvolutionReverb::prepare(double sampleRate, int newBlockSize)
{
    this->sampleRate = sampleRate;
    blockSize = newBlockSize;

    const int maximumPreDelaySamples = static_cast<int>(std::ceil(sampleRate * 0.5));
    preDelayBuffer.setSize(2, maximumPreDelaySamples + newBlockSize, false, false, true);
    preDelayBuffer.clear();
    wetBuffer.setSize(2, newBlockSize, false, false, true);
    wetBuffer.clear();
    preWritePos = 0;
    updatePreDelay();

    // Default IR: short exponential decay if none loaded
    if (!irLoaded)
    {
        juce::AudioBuffer<float> fallbackIr(1, static_cast<int>(sampleRate * 2.0));
        fallbackIr.clear();
        float* irData = fallbackIr.getWritePointer(0);
        const int irLength = fallbackIr.getNumSamples();
        for (int i = 0; i < irLength; ++i)
            irData[i] = std::exp(-3.0f * static_cast<float>(i) / static_cast<float>(sampleRate)) * 0.5f;
        convolution.loadImpulseResponse(std::move(fallbackIr),
                                        sampleRate,
                                        juce::dsp::Convolution::Stereo::no,
                                        juce::dsp::Convolution::Trim::no,
                                        juce::dsp::Convolution::Normalise::no);
        irLoaded = true;
    }

    juce::dsp::ProcessSpec spec{static_cast<double>(sampleRate), static_cast<juce::uint32>(newBlockSize), 2};
    convolution.prepare(spec);
}

void ConvolutionReverb::reset()
{
    preDelayBuffer.clear();
    wetBuffer.clear();
    preWritePos = 0;
    convolution.reset();
}

void ConvolutionReverb::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!active)
        return;

    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1 || numSamples <= 0)
        return;

    jassert(numSamples <= wetBuffer.getNumSamples());
    const int samplesToProcess = std::min(numSamples, wetBuffer.getNumSamples());

    wetBuffer.clear();
    if (numChannels == 1)
    {
        wetBuffer.copyFrom(0, 0, buffer, 0, 0, samplesToProcess);
        wetBuffer.copyFrom(1, 0, buffer, 0, 0, samplesToProcess);
    }
    else
    {
        wetBuffer.copyFrom(0, 0, buffer, 0, 0, samplesToProcess);
        wetBuffer.copyFrom(1, 0, buffer, 1, 0, samplesToProcess);
    }

    // Apply pre-delay to the wet path only.
    if (preDelaySamples > 0)
    {
        for (int i = 0; i < samplesToProcess; ++i)
        {
            const int readPos = (preWritePos - preDelaySamples + preDelayBuffer.getNumSamples()) % preDelayBuffer.getNumSamples();
            for (int ch = 0; ch < 2; ++ch)
            {
                const float delayed = preDelayBuffer.getSample(ch, readPos);
                preDelayBuffer.setSample(ch, preWritePos, wetBuffer.getSample(ch, i));
                wetBuffer.setSample(ch, i, delayed);
            }
            preWritePos = (preWritePos + 1) % preDelayBuffer.getNumSamples();
        }
    }

    auto block = juce::dsp::AudioBlock<float>(wetBuffer).getSubBlock(0, static_cast<size_t>(samplesToProcess));
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolution.process(context);

    const float irGainLinear = std::pow(10.0f, irGain / 20.0f);
    wetBuffer.applyGain(0, samplesToProcess, irGainLinear);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        const float* wet = wetBuffer.getReadPointer(ch < 2 ? ch : 0);
        for (int i = 0; i < samplesToProcess; ++i)
            data[i] = data[i] * (1.0f - mix) + wet[i] * mix;
    }
}

void ConvolutionReverb::setActive(bool active) { this->active = active; }
bool ConvolutionReverb::isActive() const { return active; }
void ConvolutionReverb::setMix(float amount) { mix = std::clamp(amount, 0.0f, 1.0f); }
void ConvolutionReverb::setPreDelayMs(float ms) { preDelayMs = std::clamp(ms, 0.0f, 500.0f); updatePreDelay(); }
void ConvolutionReverb::setIrGain(float gainDb) { irGain = std::clamp(gainDb, -24.0f, 24.0f); }

bool ConvolutionReverb::loadIrFromFile(const juce::String& filePath)
{
    juce::File file(filePath);
    if (!file.existsAsFile())
        return false;

    convolution.loadImpulseResponse(file,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::no,
                                    0,
                                    juce::dsp::Convolution::Normalise::yes);
    irLoaded = true;
    return true;
}

bool ConvolutionReverb::loadIrFromResource(std::string_view resourceName)
{
    const char* resourceNameUtf8 = nullptr;
    for (int index = 0; index < IrBinaryData::namedResourceListSize; ++index)
    {
        if (resourceName == IrBinaryData::namedResourceList[index])
        {
            resourceNameUtf8 = IrBinaryData::namedResourceList[index];
            break;
        }
    }

    if (resourceNameUtf8 == nullptr)
        return false;

    int dataSize = 0;
    const void* data = IrBinaryData::getNamedResource(resourceNameUtf8, dataSize);
    if (data == nullptr || dataSize <= 0)
        return false;

    convolution.loadImpulseResponse(data,
                                    static_cast<size_t>(dataSize),
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::no,
                                    0,
                                    juce::dsp::Convolution::Normalise::yes);
    irLoaded = true;
    return true;
}

int ConvolutionReverb::getCurrentIrSize() const { return convolution.getCurrentIRSize(); }

void ConvolutionReverb::updatePreDelay()
{
    const int requestedSamples = static_cast<int>(preDelayMs * sampleRate / 1000.0);
    const int maximumSamples = std::max(0, preDelayBuffer.getNumSamples() - 1);
    preDelaySamples = std::clamp(requestedSamples, 0, maximumSamples);
}
