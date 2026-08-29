#pragma once

#include "CommandQueue.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct DrumSampleData {
    juce::AudioBuffer<float> audio;
    double sourceSampleRate = 44100.0;
    std::string path;
    std::string name;
};

class DrumSampleLibrary {
public:
    static constexpr uint16_t kFirstUserSampleId = 1;

    struct SampleRecord {
        uint16_t id = 0;
        std::string name;
        std::string path;
        int lengthSamples = 0;
        double sourceSampleRate = 44100.0;
    };

    DrumSampleLibrary();

    bool preloadEmbeddedDrumKits(std::string& error);

    const std::string& getGlobalSampleDirectory() const;
    bool ensureGlobalSampleDirectory(std::string& error) const;
    bool rescanGlobalDirectory(std::string& error);

    bool importSampleFromPath(const std::string& sourcePath,
                              uint16_t& outSampleId,
                              std::string& error);

    std::vector<SampleRecord> listSamples() const;
    std::string getSampleName(uint16_t sampleId) const;
    std::string getSamplePath(uint16_t sampleId) const;
    bool hasSample(uint16_t sampleId) const;

    const DrumSampleData* getRtSample(uint16_t sampleId) const;

private:
    static constexpr size_t kMaxSampleIds = static_cast<size_t>(Command::kDrumSampleIdMask) + 1u;

    static bool isSupportedAudioFile(const std::string& extension);
    static std::string normalizePath(const std::string& path);
    static std::string sanitizeStem(const std::string& stem);

    uint16_t computeStableUserSampleIdLocked(const std::string& normalizedPath) const;
    bool loadSampleFromReader(juce::AudioFormatReader& reader,
                              const std::string& sourcePath,
                              const std::string& displayName,
                              std::shared_ptr<DrumSampleData>& outData,
                              std::string& error) const;
    bool loadSampleFile(const std::string& normalizedPath,
                        std::shared_ptr<DrumSampleData>& outData,
                        std::string& error);
    bool loadEmbeddedSample(const void* data,
                            size_t sizeBytes,
                            const std::string& sourcePath,
                            const std::string& displayName,
                            std::shared_ptr<DrumSampleData>& outData,
                            std::string& error);
    bool registerLoadedSampleLocked(uint16_t sampleId,
                                    const std::shared_ptr<DrumSampleData>& sampleData,
                                    const std::string& normalizedPath,
                                    const std::string& displayName,
                                    std::string& error);
    bool registerEmbeddedSample(uint16_t sampleId,
                                const void* data,
                                size_t sizeBytes,
                                const std::string& sourcePath,
                                const std::string& displayName,
                                std::string& error);
    bool registerSamplePath(const std::string& path,
                            uint16_t& outSampleId,
                            std::string& error);

    juce::AudioFormatManager formatManager;
    std::string globalSampleDirectory;

    mutable std::mutex sampleMutex;
    std::array<std::shared_ptr<DrumSampleData>, kMaxSampleIds> sampleStorage{};
    std::array<std::string, kMaxSampleIds> samplePathById{};
    std::array<std::string, kMaxSampleIds> sampleNameById{};
    std::array<std::atomic<const DrumSampleData*>, kMaxSampleIds> rtSamplePointers{};
};
