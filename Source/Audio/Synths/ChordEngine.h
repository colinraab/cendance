#pragma once
#include "SoundEngine.h"
#include "MelodicSampler.h"
#include <juce_dsp/juce_dsp.h>
#include <cstdint>

class SynthVoice : public juce::SynthesiserVoice {
public:
    SynthVoice();
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setTimbre(uint8_t preset, float tone, float motion);

private:
    juce::dsp::Oscillator<float> osc1_;
    juce::dsp::Oscillator<float> osc2_;
    juce::dsp::Oscillator<float> slowLfo_;
    juce::ADSR env_;
    juce::dsp::StateVariableTPTFilter<float> filter_;
    int currentNote_ = -1;
    float baseFreq_ = 220.0f;
    float velocity_ = 0.8f;
    uint8_t preset_ = 0;
    float tone_ = 0.5f;
    float motion_ = 0.5f;
};

class SynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class ChordEngine : public SoundEngine {
public:
    ChordEngine();

    void setPreset(uint8_t preset);
    void setTone(float tone);
    void setMotion(float motion);
    void setSampleRegion(uint8_t regionIndex, const MelodicSamplerEngine::Region& region);
    void clearSampleRegions();
    
    void prepare(double sampleRate, int blockSize) override;
    void renderNextBlock(juce::AudioBuffer<float>& buffer,
                         const juce::MidiBuffer& midi,
                         int numSamples) override;
    void reset() override;

private:
    double sampleRate_ = 44100.0;
    MelodicSamplerEngine sampler_;
    juce::Synthesiser synth_;
    uint8_t preset_ = 0;
    float tone_ = 0.5f;
    float motion_ = 0.5f;
};
