#include "StartupProjectInitializer.h"

#include "AlgorithmCatalog.h"
#include "DrumKitPresetCatalog.h"
#include "EffectPresetCatalog.h"
#include "SynthCatalog.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <optional>

namespace {

constexpr uint8_t kTrackCount = 4;
constexpr uint8_t kEffectSlotCount = 3;
constexpr float kRandomTempoMinBpm = 80.0f;
constexpr float kRandomTempoMaxBpm = 160.0f;

uint8_t randomIndexFromCount(std::mt19937& rng, uint16_t count) {
    if (count == 0) {
        return 0;
    }

    std::uniform_int_distribution<uint16_t> dist(0, static_cast<uint16_t>(count - 1));
    return static_cast<uint8_t>(dist(rng));
}

void applyDrumKitPresetToState(AppState& appState, uint8_t presetId) {
    const auto& preset = DrumKitPresetCatalog::getPreset(presetId);
    auto& drumTrack = appState.tracks[0];

    for (uint8_t slot = 0; slot < AppState::TrackState::DrumSampleSlotCount; ++slot) {
        const auto& slotConfig = preset.slots[slot];
        drumTrack.setDrumSampleSlotSampleId(slot, slotConfig.sampleId);
        drumTrack.setDrumSampleSlotVolume(slot, slotConfig.volume);
        drumTrack.setDrumSampleSlotTuneSemitones(slot, slotConfig.tuneSemitones);
        drumTrack.setDrumSampleSlotStartOffset(slot, slotConfig.startOffset);
        drumTrack.setDrumSampleSlotDecay(slot, slotConfig.decay);
        drumTrack.setDrumSampleSlotVelocitySensitivity(slot, slotConfig.velocitySensitivity);
    }
}

} // namespace

namespace StartupProjectInitializer {

void applyRandomizedStartupProject(AppState& appState, std::mt19937& rng) {
    PresetRegistry::Registry registry;
    registry.rebuild(nullptr);
    applyRandomizedStartupProject(appState, registry, rng);
}

void applyRandomizedStartupProject(AppState& appState,
                                   const PresetRegistry::Registry& presetRegistry,
                                   std::mt19937& rng) {
    std::uniform_real_distribution<float> normalizedDist(0.1f, 0.9f);
    std::uniform_real_distribution<float> tempoDist(kRandomTempoMinBpm, kRandomTempoMaxBpm);

    appState.setBpm(tempoDist(rng));

    for (uint8_t track = 0; track < kTrackCount; ++track) {
        const uint16_t algorithmCount = AlgorithmCatalog::getAlgorithmCountForTrack(track);
        const uint8_t algorithmId = randomIndexFromCount(rng, algorithmCount);
        appState.tracks[track].setAlgorithmId(algorithmId);

        const auto trackSounds = presetRegistry.soundsForTrack(track);
        const auto selectedSound = trackSounds.empty()
            ? std::optional<PresetRegistry::CatalogEntry>{}
            : std::optional<PresetRegistry::CatalogEntry>{trackSounds[std::uniform_int_distribution<size_t>(0, trackSounds.size() - 1)(rng)]};
        const auto resolvedSound = selectedSound.has_value()
            ? presetRegistry.resolveSound(selectedSound->ref)
            : std::optional<PresetRegistry::ResolvedSoundPreset>{};
        const uint16_t presetCount = SynthCatalog::getAutoSelectablePresetCountForTrack(track);
        const uint8_t presetId = resolvedSound.has_value()
            ? static_cast<uint8_t>(std::min<uint16_t>(resolvedSound->synthPresetId,
                                                      presetCount > 0 ? presetCount - 1 : 0))
            : randomIndexFromCount(rng, presetCount);
        appState.tracks[track].setSynthPreset(presetId);
        if (track == 0) {
            applyDrumKitPresetToState(appState, presetId);
        }

        appState.tracks[track].setDensity(normalizedDist(rng));
        appState.tracks[track].setComplexity(normalizedDist(rng));
        appState.tracks[track].setTone(normalizedDist(rng));
        appState.tracks[track].setMotion(normalizedDist(rng));

        // Treat randomized startup presets as intentional user-level sound choices.
        if (track > 0) {
            appState.tracks[track].setSynthManualOverride(true);
        }

        const auto effectSlots = resolvedSound.has_value()
            ? resolvedSound->fxPresetIds
            : SynthCatalog::getPresetEffectSlots(track, presetId);
        for (uint8_t slot = 0; slot < kEffectSlotCount; ++slot) {
            appState.tracks[track].setEffectPresetSlot(slot, effectSlots[slot]);
        }
    }

    const int progressionCount = ChordProgression::getNumProgressions();
    const uint8_t progressionIndex = progressionCount > 0
        ? static_cast<uint8_t>(std::uniform_int_distribution<int>(0, progressionCount - 1)(rng))
        : 0;
    appState.setChordProgression(progressionIndex);

    const uint8_t keyRoot = static_cast<uint8_t>(std::uniform_int_distribution<int>(0, 11)(rng));
    const uint8_t keyMode = static_cast<uint8_t>(
        std::uniform_int_distribution<int>(AppState::kProjectKeyModeMajor,
                                           AppState::kProjectKeyModeNaturalMinor)(rng));
    appState.setProjectKey(keyRoot, keyMode);

    for (uint8_t slot = 0; slot < kEffectSlotCount; ++slot) {
        appState.master.setEffectPresetSlot(slot, 0);
    }
    appState.master.setEffectPresetSlot(2, EffectPresetCatalog::kDefaultMasterLimiterPresetId);

    appState.setActiveSpotEffects(0);
}

void applyRandomizedStartupProject(AppState& appState) {
    std::mt19937 rng(std::random_device{}());
    applyRandomizedStartupProject(appState, rng);
}

void applyRandomizedStartupProject(AppState& appState,
                                   const PresetRegistry::Registry& presetRegistry) {
    std::mt19937 rng(std::random_device{}());
    applyRandomizedStartupProject(appState, presetRegistry, rng);
}

} // namespace StartupProjectInitializer
