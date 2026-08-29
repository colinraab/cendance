#pragma once
#include "SoundEngine.h"
#include "MelodicSampler.h"
#include <juce_dsp/juce_dsp.h>
#include <cstdint>

class BassEngine : public SoundEngine {
public:
    BassEngine();

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

    juce::dsp::Oscillator<float> oscMain_;
    juce::dsp::Oscillator<float> oscSub_;
    juce::dsp::Oscillator<float> oscDetune_;
    juce::dsp::Oscillator<float> oscFmCarrier_;
    juce::dsp::Oscillator<float> oscFmMod_;
    juce::dsp::Oscillator<float> motionLfo_;
    juce::dsp::StateVariableTPTFilter<float> filter_;
    juce::ADSR env_;
    juce::ADSR filterEnv_;
    
    int currentNote_ = -1;
    float currentFreq_ = 0.0f;
    float targetFreq_ = 0.0f;
    float currentVelocity_ = 0.8f;
    float portamentoRate_ = 0.99f;

    uint8_t preset_ = 0;
    float tone_ = 0.5f;
    float motion_ = 0.5f;
    float phaseDrift_ = 0.0f;
};
