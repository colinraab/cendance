#pragma once

#include "SoundEngine.h"

#include <array>
#include <cstdint>

class MelodicSamplerEngine : public SoundEngine {
public:
    static constexpr uint8_t RegionCount = 12;
    static constexpr uint8_t VoiceCount = 24;

    struct Region {
        const juce::AudioBuffer<float>* audio = nullptr;
        double sourceSampleRate = 44100.0;
        int rootNote = 60;
        int lowNote = 0;
        int highNote = 127;
        float gain = 1.0f;
        float tuneSemitones = 0.0f;
        float startOffset = 0.0f;
        float endOffset = 1.0f;
        bool loop = false;
        float loopStart = 0.0f;
        float loopEnd = 1.0f;
    };

    void setRegion(uint8_t regionIndex, const Region& region);
    void clearRegion(uint8_t regionIndex);
    void clearRegions();
    bool hasAssignedSamples() const;

    void setPreset(uint8_t preset);
    void setTone(float tone);
    void setMotion(float motion);

    void prepare(double sampleRate, int blockSize) override;
    void renderNextBlock(juce::AudioBuffer<float>& buffer,
                         const juce::MidiBuffer& midi,
                         int numSamples) override;
    void reset() override;

private:
    struct Voice {
        const Region* region = nullptr;
        double samplePosition = 0.0;
        double increment = 1.0;
        float gain = 1.0f;
        float envelope = 0.0f;
        float attack = 1.0f;
        float attackStep = 1.0f;
        float releaseMul = 0.9995f;
        float pan = 0.0f;
        int note = -1;
        bool active = false;
        bool releasing = false;
    };

    double sampleRate_ = 44100.0;
    uint8_t preset_ = 0;
    float tone_ = 0.5f;
    float motion_ = 0.5f;
    float lowStateL_ = 0.0f;
    float lowStateR_ = 0.0f;
    std::array<Region, RegionCount> regions_{};
    std::array<Voice, VoiceCount> voices_{};

    const Region* chooseRegionForNote(int midiNote) const;
    void triggerVoice(int midiNote, float velocity);
    void releaseVoicesForNote(int midiNote);
    void renderVoice(Voice& voice, float& leftOut, float& rightOut);
    void applyTone(float& leftOut, float& rightOut);
};
