#pragma once
#include "SoundEngine.h"
#include "../../App/DrumSampleLibrary.h"
#include <juce_dsp/juce_dsp.h>

#include <array>

class DrumEngine : public SoundEngine {
public:
    DrumEngine();

    struct SampleSlotParams {
        float volume = 1.0f;
        float tuneSemitones = 0.0f;
        float startOffset = 0.0f;
        float decay = 1.0f;
        float velocitySensitivity = 1.0f;
    };
    
    void prepare(double sampleRate, int blockSize) override;
    void renderNextBlock(juce::AudioBuffer<float>& buffer,
                         const juce::MidiBuffer& midi,
                         int numSamples) override;
    void reset() override;

    void setSampleForSlot(uint8_t slotIndex, const DrumSampleData* sampleData);
    void setSampleSlotVolume(uint8_t slotIndex, float value);
    void setSampleSlotTuneSemitones(uint8_t slotIndex, float value);
    void setSampleSlotStartOffset(uint8_t slotIndex, float value);
    void setSampleSlotDecay(uint8_t slotIndex, float value);
    void setSampleSlotVelocitySensitivity(uint8_t slotIndex, float value);
    void setTone(float tone);
    void setMotion(float motion);

private:
    static constexpr uint8_t SampleSlotCount = 4;
    static constexpr size_t SampleVoiceCount = 16;

    struct SampleVoice {
        const DrumSampleData* sampleData = nullptr;
        double samplePosition = 0.0;
        double increment = 1.0;
        float gain = 1.0f;
        float envelope = 1.0f;
        float decayMul = 0.9995f;
        float attack = 1.0f;
        float attackStep = 1.0f;
        uint8_t slotIndex = 0;
        bool active = false;
    };

    double sampleRate_ = 44100.0;

    // Kick Synth
    juce::dsp::Oscillator<float> kickOsc_;
    juce::ADSR kickEnv_;
    float kickPitchEnv_ = 0.0f;
    float kickPitchEnvDecay_ = 0.99f;

    // Snare Synth
    juce::dsp::Oscillator<float> snareOsc_;
    juce::ADSR snareEnv_;
    float snareNoiseEnv_ = 0.0f;
    float snareNoiseDecay_ = 0.999f;
    juce::Random random_;

    // Hi-hat Synth
    juce::dsp::StateVariableTPTFilter<float> hatFilter_;
    float closedHatEnv_ = 0.0f;
    float openHatEnv_ = 0.0f;
    float closedHatDecay_ = 0.995f;
    float openHatDecay_ = 0.999f;

    std::array<const DrumSampleData*, SampleSlotCount> slotSamples_{};
    std::array<SampleSlotParams, SampleSlotCount> slotParams_{};
    std::array<SampleVoice, SampleVoiceCount> sampleVoices_{};

    float tone_ = 0.5f;
    float motion_ = 0.5f;
    float tiltLowStateL_ = 0.0f;
    float tiltLowStateR_ = 0.0f;
    float tiltCoeff_ = 0.1f;
    float tiltLowGain_ = 1.0f;
    float tiltHighGain_ = 1.0f;

    void handleMidiEvent(const juce::MidiMessage& msg);
    void triggerSynthVoice(int midiNote, float velocity);
    void chokeOpenHatVoices();
    void triggerSampleVoice(uint8_t slotIndex, float velocity);
    void renderSampleVoices(float& leftOut, float& rightOut);
    void refreshMoveEnvelopeState();
    void refreshToneTiltState();
    void applyToneTilt(float& leftOut, float& rightOut);
};
