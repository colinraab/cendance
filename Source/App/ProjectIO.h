#pragma once

#include "AppState.h"
#include "CommandQueue.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ProjectIO {

constexpr uint8_t kProjectTrackCount = 4;
constexpr uint8_t kProjectEffectSlotCount = 3;
constexpr uint8_t kProjectDrumSampleSlotCount = 4;
constexpr uint8_t kProjectArrangementMaxSections = AppState::kArrangementMaxSections;
constexpr uint32_t kProjectSchemaMajor = 1;
constexpr uint32_t kProjectSchemaMinor = 8;
constexpr uint32_t kProjectForwardCompatibleSchemaMajor = 2;

struct DrumSampleSlotSnapshot {
    uint16_t sampleId = 0;
    float volume = 1.0f;
    float tuneSemitones = 0.0f;
    float startOffset = 0.0f;
    float decay = 1.0f;
    float velocitySensitivity = 1.0f;
};

struct TrackSnapshot {
    uint16_t algorithmId = 0;
    std::string customAlgorithmRef;
    uint8_t synthPreset = 0;
    std::string soundPresetRef;
    bool synthManualOverride = false;
    float density = 0.5f;
    float complexity = 0.5f;
    float tone = 0.5f;
    float motion = 0.5f;
    float gain = 1.0f;
    bool muted = false;
    std::array<uint16_t, kProjectEffectSlotCount> effectPresetSlots{{0, 0, 0}};
    std::array<std::string, kProjectEffectSlotCount> effectPresetRefs{{"", "", ""}};
    std::array<DrumSampleSlotSnapshot, kProjectDrumSampleSlotCount> drumSampleSlots{};
};

struct ProjectSnapshot {
    uint32_t schemaMajor = kProjectSchemaMajor;
    uint32_t schemaMinor = kProjectSchemaMinor;
    std::string projectName;
    std::string savedAtUtc;

    float bpm = 120.0f;
    bool playing = false;
    bool metronomeEnabled = true;
    uint8_t chordProgression = 0;
    uint8_t projectKeyRoot = 0;
    uint8_t projectKeyMode = AppState::kProjectKeyModeNaturalMinor;
    uint8_t arrangementSectionCount = 4;
    uint8_t arrangementCurrentSection = 0;
    uint8_t arrangementMode = AppState::kArrangementModeMixed;
    std::array<uint8_t, kProjectArrangementMaxSections> arrangementSectionLengths{{4, 4, 4, 4, 4, 4, 4, 4}};
    std::array<uint8_t, kProjectArrangementMaxSections> arrangementSectionProgressions{{
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
    }};
    std::array<uint8_t, kProjectArrangementMaxSections> arrangementSectionTrackMasks{{
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
        AppState::kArrangementTrackMaskAll,
    }};
    bool arrangementChainEnabled = false;
    uint8_t arrangementChainLength = AppState::kArrangementDefaultChainLength;
    std::array<uint8_t, kProjectArrangementMaxSections> arrangementChainSequence{{0, 1, 2, 3, 4, 5, 6, 7}};
    bool arrangementSectionParametersEnabled = false;
    std::array<std::array<std::array<float, AppState::kArrangementTrackParameterCount>, AppState::kTrackCount>, kProjectArrangementMaxSections>
        arrangementSectionTrackParameters{};

    std::array<TrackSnapshot, kProjectTrackCount> tracks{};
    float masterGain = 2.0f;
    std::array<uint16_t, kProjectEffectSlotCount> masterEffectPresetSlots{{0, 0, 0}};
    std::array<std::string, kProjectEffectSlotCount> masterEffectPresetRefs{{"", "", ""}};
};

ProjectSnapshot snapshotFromState(const AppState& appState);

bool validateSnapshot(const ProjectSnapshot& snapshot, std::string& error);

bool applySnapshotToCommandQueue(const ProjectSnapshot& snapshot,
                                 AppState& appState,
                                 CommandQueue& commandQueue,
                                 std::string& error,
                                 bool forceStop = true);

bool normalizeProjectPath(const std::string& inputPath,
                          std::string& normalizedPath,
                          std::string& error,
                          bool appendDefaultExtension = true);

std::string getDefaultProjectsDirectory();

std::vector<std::string> loadRecentProjectPaths(std::string& error);

bool saveRecentProjectPaths(const std::vector<std::string>& paths,
                            std::string& error);

void touchRecentProjectPath(std::vector<std::string>& recentPaths,
                            const std::string& path,
                            size_t maxEntries = 10);

std::string getRecentProjectsFilePath();

} // namespace ProjectIO
