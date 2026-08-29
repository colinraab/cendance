#pragma once

#include "../App/AppState.h"
#include "../App/ContributionPackage.h"

#include <juce_core/juce_core.h>

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Manages local save/load of arrangement presets.
// Each preset captures the full arrangement state from AppState.
// Presets are stored as JSON files in a dedicated directory.
class ArrangementPresetManager {
public:
    ArrangementPresetManager();
    ~ArrangementPresetManager();

    bool ensureDirectories(std::string& error) const;
    bool reload(std::string& error);

    // Save current arrangement state as a named preset.
    // Returns the preset ID on success (empty on failure).
    std::string savePreset(const AppState& appState, const std::string& name, std::string& error);

    // Load a preset by ID into the command queue.
    // Returns the ContributionPackage::ArrangementPresetItem for the preset.
    bool loadPreset(const std::string& presetId, AppState& appState, std::string& error) const;

    // Delete a preset by ID.
    bool deletePreset(const std::string& presetId, std::string& error);

    // List all saved presets.
    std::vector<ContributionPackage::ArrangementPresetItem> listPresets() const;

    // Find a preset by ID.
    const ContributionPackage::ArrangementPresetItem* findPreset(const std::string& presetId) const;

    // Convert AppState arrangement to ArrangementPresetItem.
    static ContributionPackage::ArrangementPresetItem snapshotFromState(const AppState& appState, const std::string& name = "");

    // Apply an ArrangementPresetItem to AppState via direct state mutation.
    // Used when loading from local presets (not via command queue).
    static void applyToState(AppState& appState, const ContributionPackage::ArrangementPresetItem& preset);

    // JSON serialization (used by both local save/load and P2P sharing)
    static std::string presetToJson(const ContributionPackage::ArrangementPresetItem& item);
    static bool presetFromJson(const std::string& json, ContributionPackage::ArrangementPresetItem& item, std::string& error);

    std::string presetsJson() const;

    juce::File rootDirectory() const { return rootDir_; }

private:
    struct PresetEntry {
        ContributionPackage::ArrangementPresetItem item;
        juce::File file;
    };

    mutable std::mutex mutex_;
    std::map<std::string, PresetEntry> presets_; // presetId -> entry
    juce::File rootDir_;

    bool loadPresetFromFile(const juce::File& file, ContributionPackage::ArrangementPresetItem& item, std::string& error) const;
    bool writePresetToFile(const ContributionPackage::ArrangementPresetItem& item, const juce::File& file, std::string& error) const;
    std::string generatePresetId(const std::string& name) const;
};

ArrangementPresetManager& globalArrangementPresetManager();
