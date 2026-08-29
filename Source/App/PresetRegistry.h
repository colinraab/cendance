#pragma once

#include "ContributionPackage.h"
#include "PresetRef.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace PresetRegistry {

constexpr uint16_t kCustomEffectPresetIdBase = 2048;

struct CatalogEntry {
    PresetRefs::PresetRef ref;
    std::string displayLabel;
    std::string name;
    std::string description;
    uint8_t trackIndex = 0;
    uint16_t runtimeId = 0;
    std::string packageName;
    std::vector<std::string> tags;
    std::array<uint16_t, 3> fxPresetIds{{0, 0, 0}};
};

struct ResolvedEffectPreset {
    PresetRefs::PresetRef ref;
    std::string name;
    EffectPresetCatalog::EffectType type = EffectPresetCatalog::EffectType::None;
    float paramA = 0.0f;
    float paramB = 0.0f;
    float paramC = 0.0f;
    uint16_t runtimePresetId = 0;
};

struct ResolvedSoundPreset {
    PresetRefs::PresetRef ref;
    std::string name;
    uint8_t trackIndex = 0;
    uint8_t synthPresetId = 0;
    std::array<uint16_t, 3> fxPresetIds{{0, 0, 0}};
    ContributionPackage::MacroDefaults macros;
};

class Registry {
public:
    void rebuild(const ContributionPackage::Library* library);

    const std::vector<CatalogEntry>& effects() const { return effectEntries; }
    const std::vector<CatalogEntry>& sounds() const { return soundEntries; }
    const std::vector<CatalogEntry>& drumKits() const { return drumKitEntries; }
    const std::vector<CatalogEntry>& arrangements() const { return arrangementEntries; }
    const std::vector<CatalogEntry>& scenes() const { return sceneEntries; }

    std::vector<CatalogEntry> soundsForTrack(uint8_t trackIndex) const;
    std::optional<CatalogEntry> findEntry(const PresetRefs::PresetRef& ref) const;
    std::optional<ResolvedEffectPreset> resolveEffect(const PresetRefs::PresetRef& ref) const;
    std::optional<ResolvedSoundPreset> resolveSound(const PresetRefs::PresetRef& ref) const;
    std::optional<uint16_t> runtimeEffectIdForRef(const PresetRefs::PresetRef& ref) const;
    std::optional<PresetRefs::PresetRef> refForRuntimeEffectId(uint16_t presetId) const;
    std::string catalogJson() const;

private:
    const ContributionPackage::Library* library = nullptr;
    std::vector<CatalogEntry> effectEntries;
    std::vector<CatalogEntry> soundEntries;
    std::vector<CatalogEntry> drumKitEntries;
    std::vector<CatalogEntry> arrangementEntries;
    std::vector<CatalogEntry> sceneEntries;
};

Registry& globalRegistry();
void publishCustomEffectsToAudio(const Registry& registry);
std::optional<ResolvedEffectPreset> resolvePublishedCustomEffect(uint16_t presetId);

PresetRefs::PresetRef builtinEffectRef(uint16_t presetId);
PresetRefs::PresetRef builtinSoundRef(uint8_t trackIndex, uint8_t presetId);
PresetRefs::PresetRef builtinDrumKitRef(uint8_t presetId);
std::optional<uint16_t> builtinEffectIdFromSlug(const std::string& slug);
std::optional<std::pair<uint8_t, uint8_t>> builtinSoundIdFromSlug(const std::string& slug);

} // namespace PresetRegistry

