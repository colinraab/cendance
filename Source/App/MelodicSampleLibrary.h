#pragma once

#include "MelodicSampleCatalog.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <array>
#include <memory>
#include <string>

class BassEngine;
class ChordEngine;
class LeadEngine;

struct MelodicSampleData {
    juce::AudioBuffer<float> audio;
    double sourceSampleRate = 44100.0;
    std::string path;
    std::string name;
};

class MelodicSampleLibrary {
public:
    bool preloadEmbeddedSamples(std::string& error);
    bool configurePreset(uint8_t trackIndex,
                         uint8_t presetId,
                         BassEngine* bassEngine,
                         ChordEngine* chordEngine,
                         LeadEngine* leadEngine,
                         std::string& error) const;

private:
    static constexpr size_t kSampleCount = 256;

    bool loadEmbeddedSample(const MelodicSampleCatalog::RegionDefinition& region,
                            std::shared_ptr<MelodicSampleData>& outData,
                            std::string& error) const;

    juce::AudioFormatManager formatManager;
    std::array<std::shared_ptr<MelodicSampleData>, kSampleCount> sampleStorage{};
    std::array<std::string_view, kSampleCount> resourceNames{};

    const MelodicSampleData* getSample(std::string_view resourceName) const;
    void clearTrack(uint8_t trackIndex,
                    BassEngine* bassEngine,
                    ChordEngine* chordEngine,
                    LeadEngine* leadEngine) const;
    void setRegion(uint8_t trackIndex,
                   uint8_t regionIndex,
                   const MelodicSamplerEngine::Region& region,
                   BassEngine* bassEngine,
                   ChordEngine* chordEngine,
                   LeadEngine* leadEngine) const;
};
