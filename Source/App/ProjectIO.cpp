#include "ProjectIO.h"

#include "AlgorithmCatalog.h"
#include "AlgorithmPresetRegistry.h"
#include "EffectPresetCatalog.h"
#include "PresetRegistry.h"
#include "SynthCatalog.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

namespace ProjectIO {

namespace {

constexpr float kFloatEpsilon = 0.0001f;

std::string trimCopy(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, (end - begin) + 1);
}

bool isNormalized01(float value) {
    return std::isfinite(value) && value >= 0.0f && value <= 1.0f;
}

bool isFiniteInRange(float value, float minValue, float maxValue) {
    return std::isfinite(value) && value >= minValue && value <= maxValue;
}

bool pushCommand(CommandQueue& commandQueue,
                 const Command& command,
                 std::string& error) {
    if (!commandQueue.push(command)) {
        error = "Command queue full while applying project. Please retry.";
        return false;
    }

    return true;
}

} // namespace

ProjectSnapshot snapshotFromState(const AppState& appState) {
    ProjectSnapshot snapshot;
    snapshot.savedAtUtc = juce::Time::getCurrentTime().toISO8601(true).toStdString();

    snapshot.bpm = appState.bpm.load(std::memory_order_relaxed);
    snapshot.playing = appState.playing.load(std::memory_order_relaxed);
    snapshot.metronomeEnabled = appState.metronomeEnabled.load(std::memory_order_relaxed);
    snapshot.chordProgression = appState.chordProgression.load(std::memory_order_relaxed);
    snapshot.projectKeyRoot = appState.projectKeyRoot.load(std::memory_order_relaxed);
    snapshot.projectKeyMode = appState.projectKeyMode.load(std::memory_order_relaxed);
    snapshot.arrangementSectionCount = appState.arrangementSectionCount.load(std::memory_order_relaxed);
    snapshot.arrangementCurrentSection = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
    snapshot.arrangementMode = appState.arrangementMode.load(std::memory_order_relaxed);
    for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
        snapshot.arrangementSectionLengths[section] = appState.getArrangementSectionLength(section);
        snapshot.arrangementSectionProgressions[section] = appState.getArrangementSectionProgression(section);
        snapshot.arrangementSectionTrackMasks[section] = appState.getArrangementSectionTrackMask(section);
    }
    snapshot.arrangementChainEnabled = appState.arrangementChainEnabled.load(std::memory_order_relaxed);
    snapshot.arrangementChainLength = appState.getArrangementChainLength();
    for (uint8_t chainStep = 0; chainStep < kProjectArrangementMaxSections; ++chainStep) {
        snapshot.arrangementChainSequence[chainStep] = appState.getArrangementChainStep(chainStep);
    }
    snapshot.arrangementSectionParametersEnabled =
        appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed);
    for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
            for (uint8_t parameter = 0; parameter < AppState::kArrangementTrackParameterCount; ++parameter) {
                snapshot.arrangementSectionTrackParameters[section][track][parameter] =
                    appState.getArrangementSectionTrackParameter(section, track, parameter);
            }
        }
    }

    for (uint8_t track = 0; track < kProjectTrackCount; ++track) {
        auto& dst = snapshot.tracks[track];
        const auto& src = appState.tracks[track];
        dst.algorithmId = src.algorithmId.load(std::memory_order_relaxed);
        dst.customAlgorithmRef.clear();
        if (dst.algorithmId >= AlgorithmPresetRegistry::kCustomAlgorithmIdBase) {
            const auto* preset = globalAlgorithmPresetRegistry().findByRuntimeId(track, dst.algorithmId);
            if (preset != nullptr)
                dst.customAlgorithmRef = makeStablePresetRef(*preset);
        }
        dst.synthPreset = src.synthPreset.load(std::memory_order_relaxed);
        dst.soundPresetRef = PresetRefs::toStableString(PresetRegistry::builtinSoundRef(track, dst.synthPreset));
        dst.synthManualOverride = src.synthManualOverride.load(std::memory_order_relaxed);
        dst.density = src.density.load(std::memory_order_relaxed);
        dst.complexity = src.complexity.load(std::memory_order_relaxed);
        dst.tone = src.tone.load(std::memory_order_relaxed);
        dst.motion = src.motion.load(std::memory_order_relaxed);
        dst.gain = src.gain.load(std::memory_order_relaxed);
        dst.muted = src.muted.load(std::memory_order_relaxed);
        for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
            dst.effectPresetSlots[slot] = src.getEffectPresetSlot(slot);
            if (const auto ref = PresetRegistry::globalRegistry().refForRuntimeEffectId(dst.effectPresetSlots[slot])) {
                dst.effectPresetRefs[slot] = PresetRefs::toStableString(*ref);
            } else {
                dst.effectPresetRefs[slot] = PresetRefs::toStableString(PresetRegistry::builtinEffectRef(dst.effectPresetSlots[slot]));
            }
        }
        for (uint8_t slot = 0; slot < kProjectDrumSampleSlotCount; ++slot) {
            auto& drumSlot = dst.drumSampleSlots[slot];
            drumSlot.sampleId = src.getDrumSampleSlotSampleId(slot);
            drumSlot.volume = src.getDrumSampleSlotVolume(slot);
            drumSlot.tuneSemitones = src.getDrumSampleSlotTuneSemitones(slot);
            drumSlot.startOffset = src.getDrumSampleSlotStartOffset(slot);
            drumSlot.decay = src.getDrumSampleSlotDecay(slot);
            drumSlot.velocitySensitivity = src.getDrumSampleSlotVelocitySensitivity(slot);
        }
    }

    for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
        snapshot.masterEffectPresetSlots[slot] = appState.master.getEffectPresetSlot(slot);
        if (const auto ref = PresetRegistry::globalRegistry().refForRuntimeEffectId(snapshot.masterEffectPresetSlots[slot])) {
            snapshot.masterEffectPresetRefs[slot] = PresetRefs::toStableString(*ref);
        } else {
            snapshot.masterEffectPresetRefs[slot] = PresetRefs::toStableString(PresetRegistry::builtinEffectRef(snapshot.masterEffectPresetSlots[slot]));
        }
    }
    snapshot.masterGain = appState.master.gain.load(std::memory_order_relaxed);

    return snapshot;
}

bool validateSnapshot(const ProjectSnapshot& snapshot, std::string& error) {
    if (snapshot.schemaMajor != kProjectSchemaMajor) {
        error = "Unsupported project schema major version.";
        return false;
    }

    if (!std::isfinite(snapshot.bpm) || snapshot.bpm < 20.0f || snapshot.bpm > 260.0f) {
        error = "BPM must be between 20 and 260.";
        return false;
    }

    if (snapshot.chordProgression >= static_cast<uint8_t>(ChordProgression::getNumProgressions())) {
        error = "Chord progression index is out of range.";
        return false;
    }

    if (snapshot.projectKeyRoot >= 12) {
        error = "Project key root is out of range.";
        return false;
    }

    if (snapshot.projectKeyMode != AppState::kProjectKeyModeMajor
        && snapshot.projectKeyMode != AppState::kProjectKeyModeNaturalMinor) {
        error = "Project key mode is invalid.";
        return false;
    }

    if (snapshot.arrangementSectionCount < 1 || snapshot.arrangementSectionCount > kProjectArrangementMaxSections) {
        error = "Arrangement section count is out of range.";
        return false;
    }

    if (snapshot.arrangementCurrentSection >= snapshot.arrangementSectionCount) {
        error = "Arrangement current section is out of range.";
        return false;
    }

    if (snapshot.arrangementMode > AppState::kArrangementModeMixed) {
        error = "Arrangement mode is invalid.";
        return false;
    }

    for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
        if (!isFiniteInRange(static_cast<float>(snapshot.arrangementSectionLengths[section]),
                             static_cast<float>(AppState::kArrangementMinSectionLengthBars),
                             static_cast<float>(AppState::kArrangementMaxSectionLengthBars))) {
            error = "Arrangement section length is out of range.";
            return false;
        }

        const uint8_t progression = snapshot.arrangementSectionProgressions[section];
        if (progression != AppState::kArrangementProgressionFollowGlobal
            && progression >= static_cast<uint8_t>(ChordProgression::getNumProgressions())) {
            error = "Arrangement section progression override is out of range.";
            return false;
        }

        if ((snapshot.arrangementSectionTrackMasks[section] & ~AppState::kArrangementTrackMaskAll) != 0u) {
            error = "Arrangement section track mask is invalid.";
            return false;
        }
    }

    if (snapshot.arrangementChainLength < 1 || snapshot.arrangementChainLength > kProjectArrangementMaxSections) {
        error = "Arrangement chain length is out of range.";
        return false;
    }

    for (uint8_t chainStep = 0; chainStep < kProjectArrangementMaxSections; ++chainStep) {
        const uint8_t section = snapshot.arrangementChainSequence[chainStep];
        if (section >= kProjectArrangementMaxSections) {
            error = "Arrangement chain section index is out of range.";
            return false;
        }
        if (snapshot.arrangementChainEnabled
            && chainStep < snapshot.arrangementChainLength
            && section >= snapshot.arrangementSectionCount) {
            error = "Arrangement chain section index exceeds section count.";
            return false;
        }
    }

    for (uint8_t track = 0; track < kProjectTrackCount; ++track) {
        const auto& trackSnapshot = snapshot.tracks[track];
        const uint16_t builtinCount = AlgorithmCatalog::getAlgorithmCountForTrack(track);
        bool algoValid = (trackSnapshot.algorithmId < builtinCount);
        if (!algoValid && trackSnapshot.algorithmId >= AlgorithmPresetRegistry::kCustomAlgorithmIdBase) {
            // Validate custom algorithm exists in registry
            const auto* preset = globalAlgorithmPresetRegistry().findByRuntimeId(track, trackSnapshot.algorithmId);
            algoValid = (preset != nullptr);
        }
        if (!algoValid) {
            error = "Track algorithm index is out of range.";
            return false;
        }

        const uint16_t synthCount = SynthCatalog::getPresetCountForTrack(track);
        if (synthCount == 0 || trackSnapshot.synthPreset >= synthCount) {
            error = "Track synth preset index is out of range.";
            return false;
        }

        if (!isNormalized01(trackSnapshot.density)
            || !isNormalized01(trackSnapshot.complexity)
            || !isNormalized01(trackSnapshot.tone)
            || !isNormalized01(trackSnapshot.motion)) {
            error = "Track parameters must be in [0, 1].";
            return false;
        }

        if (!std::isfinite(trackSnapshot.gain) || trackSnapshot.gain < 0.0f || trackSnapshot.gain > 2.0f) {
            error = "Track gain must be in [0, 2].";
            return false;
        }

        for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
            const bool hasValidRef = !trackSnapshot.effectPresetRefs[slot].empty()
                && PresetRefs::parseStableString(trackSnapshot.effectPresetRefs[slot]).has_value();
            if (!EffectPresetCatalog::isValidPresetId(trackSnapshot.effectPresetSlots[slot]) && !hasValidRef) {
                error = "Track effect preset is invalid.";
                return false;
            }
        }

        for (uint8_t slot = 0; slot < kProjectDrumSampleSlotCount; ++slot) {
            const auto& drumSlot = trackSnapshot.drumSampleSlots[slot];
            if (drumSlot.sampleId > Command::kDrumSampleIdMask) {
                error = "Drum sample assignment ID is out of range.";
                return false;
            }

            if (!isFiniteInRange(drumSlot.volume, 0.0f, 2.0f)
                || !isFiniteInRange(drumSlot.tuneSemitones, -24.0f, 24.0f)
                || !isFiniteInRange(drumSlot.startOffset, 0.0f, 0.95f)
                || !isNormalized01(drumSlot.decay)
                || !isNormalized01(drumSlot.velocitySensitivity)) {
                error = "Drum sample slot parameters are out of range.";
                return false;
            }
        }
    }

    for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
        const bool hasValidRef = !snapshot.masterEffectPresetRefs[slot].empty()
            && PresetRefs::parseStableString(snapshot.masterEffectPresetRefs[slot]).has_value();
        if (!EffectPresetCatalog::isValidPresetId(snapshot.masterEffectPresetSlots[slot]) && !hasValidRef) {
            error = "Master effect preset is invalid.";
            return false;
        }
    }

    if (!std::isfinite(snapshot.masterGain) || snapshot.masterGain < 0.0f || snapshot.masterGain > AppState::kMaxMasterGain) {
        error = "Master gain must be in [0, 4].";
        return false;
    }

    return true;
}

bool applySnapshotToCommandQueue(const ProjectSnapshot& snapshot,
                                 AppState& appState,
                                 CommandQueue& commandQueue,
                                 std::string& error,
                                 bool forceStop) {
    error.clear();

    std::string validationError;
    if (!validateSnapshot(snapshot, validationError)) {
        error = validationError;
        return false;
    }

    if (forceStop) {
        if (!pushCommand(commandQueue, Command{Command::Type::Stop, 0, 0, 0.0f}, error)) {
            return false;
        }
    }

    const float currentBpm = appState.bpm.load(std::memory_order_relaxed);
    const float bpmDelta = snapshot.bpm - currentBpm;
    if (std::fabs(bpmDelta) > kFloatEpsilon) {
        if (!pushCommand(commandQueue, Command{Command::Type::SetTempo, 0, 0, bpmDelta}, error)) {
            return false;
        }
    }

    const bool currentMetronome = appState.metronomeEnabled.load(std::memory_order_relaxed);
    if (currentMetronome != snapshot.metronomeEnabled) {
        if (!pushCommand(commandQueue, Command{Command::Type::ToggleMetronome, 0, 0, 0.0f}, error)) {
            return false;
        }
    }

    const uint16_t keyPayload = Command::encodeProjectKey(snapshot.projectKeyRoot, snapshot.projectKeyMode);
    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetProjectKey, 0, keyPayload, 0.0f},
                     error)) {
        return false;
    }

    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetChordProg, 0, snapshot.chordProgression, 0.0f},
                     error)) {
        return false;
    }

    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetArrangementSectionCount,
                             0,
                             snapshot.arrangementSectionCount,
                             0.0f},
                     error)) {
        return false;
    }

    const uint8_t targetArrangementMode = static_cast<uint8_t>(std::min<uint8_t>(
        snapshot.arrangementMode,
        AppState::kArrangementModeMixed));
    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetArrangementMode, 0, targetArrangementMode, 0.0f},
                     error)) {
        return false;
    }

    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetArrangementSectionParametersEnabled,
                             0,
                             static_cast<uint16_t>(snapshot.arrangementSectionParametersEnabled ? 1u : 0u),
                             0.0f},
                     error)) {
        return false;
    }

    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetArrangementChainEnabled,
                             0,
                             static_cast<uint16_t>(snapshot.arrangementChainEnabled ? 1u : 0u),
                             0.0f},
                     error)) {
        return false;
    }

    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetArrangementChainLength,
                             0,
                             snapshot.arrangementChainLength,
                             0.0f},
                     error)) {
        return false;
    }

    for (uint8_t chainStep = 0; chainStep < kProjectArrangementMaxSections; ++chainStep) {
        const uint16_t chainPayload = Command::encodeArrangementSectionValue(
            chainStep,
            snapshot.arrangementChainSequence[chainStep]);
        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetArrangementChainStep, 0, chainPayload, 0.0f},
                         error)) {
            return false;
        }
    }

    for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
        const uint16_t lengthPayload = Command::encodeArrangementSectionValue(
            section,
            snapshot.arrangementSectionLengths[section]);
        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetArrangementSectionLength, 0, lengthPayload, 0.0f},
                         error)) {
            return false;
        }

        const uint16_t progressionPayload = Command::encodeArrangementSectionValue(
            section,
            snapshot.arrangementSectionProgressions[section]);
        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetArrangementSectionProgression, 0, progressionPayload, 0.0f},
                         error)) {
            return false;
        }

        const uint16_t trackMaskPayload = Command::encodeArrangementSectionValue(
            section,
            snapshot.arrangementSectionTrackMasks[section]);
        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetArrangementSectionTrackMask, 0, trackMaskPayload, 0.0f},
                         error)) {
            return false;
        }

        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
            for (uint8_t parameter = 0; parameter < AppState::kArrangementTrackParameterCount; ++parameter) {
                const uint8_t encodedTrack = static_cast<uint8_t>(track + parameter * AppState::kTrackCount);
                if (!pushCommand(commandQueue,
                                 Command{Command::Type::SetArrangementSectionTrackParameter,
                                         encodedTrack,
                                         section,
                                         snapshot.arrangementSectionTrackParameters[section][track][parameter]},
                                 error)) {
                    return false;
                }
            }
        }
    }

    if (!pushCommand(commandQueue,
                     Command{Command::Type::SetArrangementSection,
                             0,
                             snapshot.arrangementCurrentSection,
                             0.0f},
                     error)) {
        return false;
    }

    // Spot trigger state is runtime-only and intentionally resets on load.
    appState.setActiveSpotEffects(0);
    for (uint8_t track = 0; track < kProjectTrackCount; ++track) {
        appState.tracks[track].setSynthManualOverride(snapshot.tracks[track].synthManualOverride);
    }

    for (uint8_t track = 0; track < kProjectTrackCount; ++track) {
        const auto& target = snapshot.tracks[track];
        const auto& current = appState.tracks[track];

        uint16_t targetAlgoId = target.algorithmId;
        if (targetAlgoId >= AlgorithmPresetRegistry::kCustomAlgorithmIdBase && !target.customAlgorithmRef.empty()) {
            // Resolve custom algorithm ref to current runtime ID
            // Format: "local.algorithms/<presetId>/<version>"
            std::string ref = target.customAlgorithmRef;
            std::string prefix = "local.algorithms/";
            if (ref.find(prefix) == 0) {
                std::string rest = ref.substr(prefix.size());
                // Extract presetId (before the version slash)
                auto slashPos = rest.find('/');
                std::string presetId = (slashPos != std::string::npos) ? rest.substr(0, slashPos) : rest;
                const auto* preset = globalAlgorithmPresetRegistry().findByPresetId(presetId);
                if (preset != nullptr) {
                    auto runtimeId = globalAlgorithmPresetRegistry().runtimeIdForPresetId(track, preset->id);
                    if (runtimeId.has_value())
                        targetAlgoId = runtimeId.value();
                }
            }
            // If not found, fall back to built-in 0 (graceful degradation)
            if (targetAlgoId >= AlgorithmPresetRegistry::kCustomAlgorithmIdBase) {
                const auto* checkPreset = globalAlgorithmPresetRegistry().findByRuntimeId(track, targetAlgoId);
                if (checkPreset == nullptr)
                    targetAlgoId = 0;
            }
        }

        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetAlgorithm, track, targetAlgoId, 0.0f},
                         error)) {
            return false;
        }

        if (track == 0 || (track > 0 && target.synthManualOverride)) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetSynthPreset, track, target.synthPreset, 0.0f},
                             error)) {
                return false;
            }
        }

        const float densityDelta = target.density - current.density.load(std::memory_order_relaxed);
        if (std::fabs(densityDelta) > kFloatEpsilon) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetDensity, track, 0, densityDelta},
                             error)) {
                return false;
            }
        }

        const float complexityDelta = target.complexity - current.complexity.load(std::memory_order_relaxed);
        if (std::fabs(complexityDelta) > kFloatEpsilon) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetComplexity, track, 0, complexityDelta},
                             error)) {
                return false;
            }
        }

        const float toneDelta = target.tone - current.tone.load(std::memory_order_relaxed);
        if (std::fabs(toneDelta) > kFloatEpsilon) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetTone, track, 0, toneDelta},
                             error)) {
                return false;
            }
        }

        const float motionDelta = target.motion - current.motion.load(std::memory_order_relaxed);
        if (std::fabs(motionDelta) > kFloatEpsilon) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetMotion, track, 0, motionDelta},
                             error)) {
                return false;
            }
        }

        const float gainDelta = target.gain - current.gain.load(std::memory_order_relaxed);
        if (std::fabs(gainDelta) > kFloatEpsilon) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetTrackGain, track, 0, gainDelta},
                             error)) {
                return false;
            }
        }

        const bool currentlyMuted = current.muted.load(std::memory_order_relaxed);
        if (currentlyMuted != target.muted) {
            if (!pushCommand(commandQueue,
                             Command{Command::Type::ToggleTrackMute, track, 0, 0.0f},
                             error)) {
                return false;
            }
        }

        for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
            uint16_t presetId = target.effectPresetSlots[slot];
            if (!target.effectPresetRefs[slot].empty()) {
                if (const auto ref = PresetRefs::parseStableString(target.effectPresetRefs[slot])) {
                    if (const auto runtimeId = PresetRegistry::globalRegistry().runtimeEffectIdForRef(*ref)) {
                        presetId = *runtimeId;
                    }
                }
            }
            const uint16_t payload = Command::encodeEffectSlotPreset(slot, presetId);
            if (!pushCommand(commandQueue,
                             Command{Command::Type::SetTrackEffectPreset, track, payload, 0.0f},
                             error)) {
                return false;
            }
        }

        if (track == 0) {
            for (uint8_t slot = 0; slot < kProjectDrumSampleSlotCount; ++slot) {
                const auto& drumSlot = target.drumSampleSlots[slot];
                if (drumSlot.sampleId == 0) {
                    if (!pushCommand(commandQueue,
                                     Command{Command::Type::ClearDrumSampleAssignment, track, slot, 0.0f},
                                     error)) {
                        return false;
                    }
                } else {
                    const uint16_t payload = Command::encodeDrumSlotSampleId(slot, drumSlot.sampleId);
                    if (!pushCommand(commandQueue,
                                     Command{Command::Type::SetDrumSampleAssignment, track, payload, 0.0f},
                                     error)) {
                        return false;
                    }
                }

                if (!pushCommand(commandQueue,
                                 Command{Command::Type::SetDrumSampleVolume, track, slot, drumSlot.volume},
                                 error)
                    || !pushCommand(commandQueue,
                                    Command{Command::Type::SetDrumSampleTune, track, slot, drumSlot.tuneSemitones},
                                    error)
                    || !pushCommand(commandQueue,
                                    Command{Command::Type::SetDrumSampleStartOffset, track, slot, drumSlot.startOffset},
                                    error)
                    || !pushCommand(commandQueue,
                                    Command{Command::Type::SetDrumSampleDecay, track, slot, drumSlot.decay},
                                    error)
                    || !pushCommand(commandQueue,
                                    Command{Command::Type::SetDrumSampleVelocitySensitivity, track, slot, drumSlot.velocitySensitivity},
                                    error)) {
                    return false;
                }
            }
        }
    }

    for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
        uint16_t presetId = snapshot.masterEffectPresetSlots[slot];
        if (!snapshot.masterEffectPresetRefs[slot].empty()) {
            if (const auto ref = PresetRefs::parseStableString(snapshot.masterEffectPresetRefs[slot])) {
                if (const auto runtimeId = PresetRegistry::globalRegistry().runtimeEffectIdForRef(*ref)) {
                    presetId = *runtimeId;
                }
            }
        }
        const uint16_t payload = Command::encodeEffectSlotPreset(slot, presetId);
        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetMasterEffectPreset, 0, payload, 0.0f},
                         error)) {
            return false;
        }
    }

    const float masterGainDelta = snapshot.masterGain - appState.master.gain.load(std::memory_order_relaxed);
    if (std::fabs(masterGainDelta) > kFloatEpsilon) {
        if (!pushCommand(commandQueue,
                         Command{Command::Type::SetTrackGain, AppState::kTrackCount, 0, masterGainDelta},
                         error)) {
            return false;
        }
    }

    if (forceStop) {
        if (!pushCommand(commandQueue, Command{Command::Type::Stop, 0, 0, 0.0f}, error)) {
            return false;
        }
    }

    return true;
}

bool normalizeProjectPath(const std::string& inputPath,
                          std::string& normalizedPath,
                          std::string& error,
                          bool appendDefaultExtension) {
    error.clear();
    normalizedPath.clear();

    const std::string trimmed = trimCopy(inputPath);
    if (trimmed.empty()) {
        error = "Project path cannot be empty.";
        return false;
    }

    try {
        std::filesystem::path path(trimmed);
        if (appendDefaultExtension && path.extension().empty()) {
            path.replace_extension(".cendance");
        }

        if (path.is_relative()) {
            path = std::filesystem::path(getDefaultProjectsDirectory()) / path;
        }

        normalizedPath = path.lexically_normal().string();
        return true;
    } catch (const std::exception& ex) {
        error = std::string("Invalid project path: ") + ex.what();
        return false;
    }
}

std::string getDefaultProjectsDirectory() {
#if defined(_WIN32)
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif

    if (home != nullptr && home[0] != '\0') {
        return (std::filesystem::path(home) / "Documents" / "cendance").string();
    }

    return (std::filesystem::current_path() / "cendance").string();
}

std::string getRecentProjectsFilePath() {
    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        return (std::filesystem::path(home) / ".cendance_recent_projects.json").string();
    }

    return (std::filesystem::current_path() / ".cendance_recent_projects.json").string();
}

std::vector<std::string> loadRecentProjectPaths(std::string& error) {
    error.clear();
    std::vector<std::string> paths;

    const std::string recentsFile = getRecentProjectsFilePath();
    std::ifstream input(recentsFile, std::ios::binary);
    if (!input.is_open()) {
        return paths;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();

    juce::var parsed;
    const juce::Result parseResult = juce::JSON::parse(buffer.str(), parsed);
    if (parseResult.failed()) {
        error = "Failed to parse recent projects file.";
        return {};
    }

    auto* root = parsed.getDynamicObject();
    if (root == nullptr) {
        error = "Recent projects file root is invalid.";
        return {};
    }

    auto* recentArray = root->getProperty("recentProjects").getArray();
    if (recentArray == nullptr) {
        return paths;
    }

    for (const auto& entry : *recentArray) {
        const std::string item = trimCopy(entry.toString().toStdString());
        if (!item.empty()) {
            paths.push_back(item);
        }
    }

    return paths;
}

bool saveRecentProjectPaths(const std::vector<std::string>& paths,
                            std::string& error) {
    error.clear();

    const std::string recentsFile = getRecentProjectsFilePath();
    try {
        const std::filesystem::path filePath(recentsFile);
        if (filePath.has_parent_path()) {
            std::filesystem::create_directories(filePath.parent_path());
        }
    } catch (const std::exception& ex) {
        error = std::string("Unable to prepare recent projects directory: ") + ex.what();
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    juce::Array<juce::var> recentArray;
    for (const auto& path : paths) {
        recentArray.add(juce::String(path));
    }
    root->setProperty("recentProjects", juce::var(recentArray));

    const juce::String json = juce::JSON::toString(juce::var(root.release()), true);
    std::ofstream output(recentsFile, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        error = "Unable to write recent projects file.";
        return false;
    }

    output << json.toStdString();
    if (!output.good()) {
        error = "Failed while writing recent projects file.";
        return false;
    }

    return true;
}

void touchRecentProjectPath(std::vector<std::string>& recentPaths,
                            const std::string& path,
                            size_t maxEntries) {
    if (path.empty()) {
        return;
    }

    recentPaths.erase(std::remove(recentPaths.begin(), recentPaths.end(), path), recentPaths.end());
    recentPaths.insert(recentPaths.begin(), path);

    if (recentPaths.size() > maxEntries) {
        recentPaths.resize(maxEntries);
    }
}


} // namespace ProjectIO
