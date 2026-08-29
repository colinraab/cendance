#pragma once

#include "CustomAlgorithmPreset.h"
#include <juce_core/juce_core.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class AlgorithmPresetRegistry {
public:
    static constexpr uint16_t kCustomAlgorithmIdBase = 2048;

    AlgorithmPresetRegistry();
    ~AlgorithmPresetRegistry();

    bool ensureDirectories(std::string& error) const;
    bool reload(std::string& error);
    bool savePreset(const CustomAlgorithmPreset& preset, std::string& error);
    bool updatePreset(uint16_t algorithmId, const CustomAlgorithmPreset& preset, std::string& error);
    bool deletePreset(uint16_t algorithmId, std::string& error);

    const CustomAlgorithmPreset* findByRuntimeId(uint8_t trackIndex, uint16_t algorithmId) const;
    const CustomAlgorithmPreset* findByPresetId(const std::string& id) const;
    std::vector<CustomAlgorithmPreset> listForTrack(uint8_t trackIndex) const;
    std::vector<CustomAlgorithmPreset> listByGenre(uint8_t trackIndex, uint8_t genreId) const;
    std::optional<uint16_t> runtimeIdForPresetId(uint8_t trackIndex, const std::string& id) const;

    std::string catalogJson() const;

    juce::File rootDirectory() const;
    juce::File indexFile() const;

    uint16_t getCustomAlgorithmCountForTrack(uint8_t trackIndex) const;
    uint16_t getTotalAlgorithmCountForTrack(uint8_t trackIndex) const;

    bool isCustomAlgorithmId(uint16_t algorithmId) const;
    uint16_t customRuntimeIndex(uint16_t algorithmId) const;
    std::string algorithmMetadataJson(uint8_t trackIndex, uint16_t algorithmId) const;

    void rebuildIndex();

private:
    struct PresetEntry {
        CustomAlgorithmPreset preset;
        uint16_t runtimeId = 0;
        juce::File file;
    };

    mutable std::mutex mutex_;
    std::array<std::vector<PresetEntry>, 4> trackPresets_;
    std::map<std::string, std::pair<uint8_t, uint16_t>> presetIdIndex_; // id -> (trackIndex, runtimeId)

    juce::File rootDir_;
    juce::File indexFile_;

    void loadIndexFromDisk();
    bool writeIndexToDisk(std::string& error) const;
    bool loadPresetFromFile(const juce::File& file, CustomAlgorithmPreset& preset, std::string& error) const;
    bool writePresetToFile(const CustomAlgorithmPreset& preset, const juce::File& file, std::string& error) const;
    void assignRuntimeIds();
};

AlgorithmPresetRegistry& globalAlgorithmPresetRegistry();
