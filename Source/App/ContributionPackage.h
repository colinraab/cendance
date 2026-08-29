#pragma once

#include "AppState.h"
#include "CommandQueue.h"
#include "DrumKitPresetCatalog.h"
#include "EffectPresetCatalog.h"
#include "PresetRef.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ContributionPackage {

constexpr const char* kSchema = "cendancePackage.v1";

enum class Kind : uint8_t {
    EffectPresetPack = 0,
    SoundPresetPack,
    DrumKitPresetPack,
    ScenePresetPack,
    ArrangementPresetPack,
    SamplePack,
};

struct Compatibility {
    std::string minAppVersion;
    std::string maxAppVersion;
    std::string minPackageSchema = kSchema;
    std::string maxPackageSchema = kSchema;
};

struct FxSlotRef {
    uint8_t slot = 0;
    uint16_t effectPresetId = 0;
    std::optional<PresetRefs::PresetRef> effectPresetRef;
};

struct MacroDefaults {
    std::optional<float> density;
    std::optional<float> complexity;
    std::optional<float> tone;
    std::optional<float> motion;
    std::optional<float> gain;
};

struct EffectPresetItem {
    std::string itemId;
    std::string name;
    std::string description;
    EffectPresetCatalog::EffectType type = EffectPresetCatalog::EffectType::None;
    float paramA = 0.0f;
    float paramB = 0.0f;
    float paramC = 0.0f;
    std::optional<uint16_t> basedOnEffectPresetId;
    std::optional<PresetRefs::PresetRef> basedOnEffectRef;
    std::vector<std::string> tags;
};

struct SoundPresetItem {
    std::string itemId;
    std::string name;
    std::string description;
    uint8_t trackIndex = 0;
    uint8_t synthPresetId = 0;
    std::optional<PresetRefs::PresetRef> soundRef;
    std::array<uint16_t, 3> fxPresetIds{{0, 0, 0}};
    std::array<std::optional<PresetRefs::PresetRef>, 3> fxPresetRefs{};
    MacroDefaults macros;
    std::vector<std::string> tags;
};

struct DrumSlot {
    uint16_t sampleId = 0;
    float volume = 1.0f;
    float tuneSemitones = 0.0f;
    float startOffset = 0.0f;
    float decay = 1.0f;
    float velocitySensitivity = 1.0f;
};

struct DrumKitPresetItem {
    std::string itemId;
    std::string name;
    std::string description;
    std::array<DrumSlot, DrumKitPresetCatalog::kPresets[0].slots.size()> slots{};
    std::array<uint16_t, 3> fxPresetIds{{0, 0, 0}};
    std::array<std::optional<PresetRefs::PresetRef>, 3> fxPresetRefs{};
    std::vector<std::string> tags;
};

struct TrackScene {
    uint8_t algorithmId = 0;
    uint8_t synthPresetId = 0;
    std::optional<PresetRefs::PresetRef> soundRef;
    MacroDefaults macros;
    bool hasMuted = false;
    bool muted = false;
    std::array<uint16_t, 3> fxPresetIds{{0, 0, 0}};
    std::array<std::optional<PresetRefs::PresetRef>, 3> fxPresetRefs{};
};

struct ArrangementPresetItem {
    std::string itemId;
    std::string name;
    std::string description;
    uint8_t sectionCount = 4;
    uint8_t currentSection = 0;
    uint8_t mode = AppState::kArrangementModeMixed;
    std::array<uint8_t, AppState::kArrangementMaxSections> sectionLengths{{4, 4, 4, 4, 4, 4, 4, 4}};
    std::array<uint8_t, AppState::kArrangementMaxSections> sectionProgressions{{
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
    }};
    std::array<uint8_t, AppState::kArrangementMaxSections> trackMasks{{
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
    }};
    bool chainEnabled = false;
    uint8_t chainLength = AppState::kArrangementDefaultChainLength;
    std::array<uint8_t, AppState::kArrangementMaxSections> chainSequence{{0, 1, 2, 3, 4, 5, 6, 7}};
    std::vector<std::string> tags;
};

struct ScenePresetItem {
    std::string itemId;
    std::string name;
    std::string description;
    std::optional<float> bpm;
    std::optional<uint8_t> chordProgression;
    std::optional<uint8_t> projectKeyRoot;
    std::optional<uint8_t> projectKeyMode;
    std::array<TrackScene, AppState::kTrackCount> tracks{};
    std::array<bool, AppState::kTrackCount> hasTrack{{false, false, false, false}};
    std::array<uint16_t, 3> masterFxPresetIds{{0, 0, 0}};
    std::array<std::optional<PresetRefs::PresetRef>, 3> masterFxPresetRefs{};
    bool hasMasterFx = false;
    std::optional<float> masterGain;
    std::optional<ArrangementPresetItem> arrangement;
    std::vector<std::string> tags;
};

struct SamplePackItem {
    std::string itemId;
    std::string name;
    std::string description;
    std::string filePath;
    std::string format;
    std::string sha256;
    uint32_t sampleRate = 0;
    uint16_t channels = 0;
    double duration = 0.0;
    std::vector<std::string> tags;
};

struct Package {
    std::string schema = kSchema;
    std::string packageId;
    std::string version;
    Kind kind = Kind::SoundPresetPack;
    std::string name;
    std::string description;
    std::string authorAgent;
    std::string createdAt;
    std::string license;
    Compatibility compatibility;
    std::string contentHash;
    std::string signature;
    std::vector<std::string> dependencies;
    std::vector<std::string> tags;
    std::string sourcePath;
    std::string installedPath;
    bool installed = false;
    bool signaturePresent = false;
    bool hashVerified = false;

    std::vector<EffectPresetItem> effectPresets;
    std::vector<SoundPresetItem> soundPresets;
    std::vector<DrumKitPresetItem> drumKitPresets;
    std::vector<ArrangementPresetItem> arrangementPresets;
    std::vector<ScenePresetItem> scenePresets;
    std::vector<SamplePackItem> samplePacks;
};

struct Preview {
    bool ok = false;
    Package package;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

std::string kindToString(Kind kind);
std::optional<Kind> kindFromString(const std::string& text);

class Library {
public:
    Library();

    const std::string& getRootDirectory() const { return rootDirectory; }
    const std::string& getStagingDirectory() const { return stagingDirectory; }
    const std::string& getInstalledDirectory() const { return installedDirectory; }
    const std::string& getPayloadDirectory() const { return payloadDirectory; }

    bool ensureDirectories(std::string& error) const;
    Preview previewFile(const std::string& path) const;
    bool installFile(const std::string& path, Preview& preview, std::string& error);
    bool removePackage(const std::string& packageId, std::string& error);
    bool reloadInstalled(std::string& error);
    bool exportPackageTemplate(const std::string& path,
                               Kind kind,
                               const std::string& packageId,
                               const std::string& name,
                               std::string& error) const;

    const std::vector<Package>& installedPackages() const { return packages; }
    const Package* findPackage(const std::string& packageId) const;
    const SoundPresetItem* findSoundPreset(const std::string& packageId, const std::string& itemId) const;
    const EffectPresetItem* findEffectPreset(const std::string& packageId, const std::string& itemId) const;
    const DrumKitPresetItem* findDrumKitPreset(const std::string& packageId, const std::string& itemId) const;
    const ArrangementPresetItem* findArrangementPreset(const std::string& packageId, const std::string& itemId) const;
    const ScenePresetItem* findScenePreset(const std::string& packageId, const std::string& itemId) const;
    const SamplePackItem* findSamplePackItem(const std::string& packageId, const std::string& itemId) const;

    std::string packagesJson(bool includeItems) const;
    std::string contributionCatalogJson() const;
    std::string previewJson(const Preview& preview) const;

private:
    std::string rootDirectory;
    std::string stagingDirectory;
    std::string installedDirectory;
    std::string payloadDirectory;
    std::vector<Package> packages;
};

} // namespace ContributionPackage
