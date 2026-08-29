#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class MasterEffect {
public:
    virtual ~MasterEffect() = default;
    
    // Initialise the effect
    virtual void prepare(double sampleRate, int blockSize) = 0;
    
    // Process audio block directly
    virtual void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) = 0;
    
    // Momentary trigger for effect
    virtual void setActive(bool active) = 0;
    virtual bool isActive() const = 0;
    
    // Reset state
    virtual void reset() = 0;

    // Optional override for effects that need to know current tempo
    virtual void setBpm(float bpm) {}
};
