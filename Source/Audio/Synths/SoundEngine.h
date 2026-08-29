#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

class SoundEngine {
public:
    virtual ~SoundEngine() = default;
    
    virtual void prepare(double sampleRate, int blockSize) = 0;
    
    virtual void renderNextBlock(juce::AudioBuffer<float>& buffer,
                                 const juce::MidiBuffer& midi,
                                 int numSamples) = 0;
                                 
    virtual void reset() = 0;
};
