#include "CommandProcessor.h"

#include "EffectProcessor.h"
#include "../App/AlgorithmCatalog.h"
#include "../App/GenreCatalog.h"
#include "../App/SpotEffectCatalog.h"
#include "../App/SynthCatalog.h"
#include "Harmony/ChordProgression.h"

#include <algorithm>
#include <cstdlib>

CommandProcessor::CommandProcessor(AppState& appState,
                                   CommandQueue& commandQueue,
                                   EffectProcessor& effectProcessor,
                                   AlgorithmPresetRegistry* algorithmRegistry,
                                   DrumSampleLibrary* drumSampleLibrary,
                                   Delegate& delegate)
    : appState(appState)
    , commandQueue(commandQueue)
    , effectProcessor(effectProcessor)
    , algorithmRegistry(algorithmRegistry)
    , drumSampleLibrary(drumSampleLibrary)
    , delegate(delegate)
{
}

void CommandProcessor::rebuildCustomAlgorithmInstances()
{
    if (algorithmRegistry == nullptr)
        return;

    for (uint8_t track = 0; track < TrackCount; ++track)
    {
        customTrackAlgorithms[track].clear();
        auto presets = algorithmRegistry->listForTrack(track);
        for (auto& preset : presets)
        {
            customTrackAlgorithms[track].push_back(
                std::make_unique<CustomAlgorithmInstance>(preset));
        }
        customAlgorithmCounts[track] = static_cast<uint16_t>(customTrackAlgorithms[track].size());
    }
}

uint16_t CommandProcessor::getMaxAlgorithmIdForTrack(uint8_t trackIndex) const
{
    if (trackIndex >= TrackCount)
        return 0;

    uint16_t maxId = static_cast<uint16_t>(AlgorithmCatalog::kAlgorithmsPerTrack - 1);
    uint16_t customCount = customAlgorithmCounts[trackIndex];
    if (customCount > 0)
    {
        uint16_t maxCustom = static_cast<uint16_t>(
            AlgorithmPresetRegistry::kCustomAlgorithmIdBase + customCount - 1);
        if (maxCustom > maxId) maxId = maxCustom;
    }
    return maxId;
}

GenerativeAlgorithm* CommandProcessor::getTrackAlgorithm(uint8_t trackIndex, uint16_t algorithmId,
    const std::array<std::array<GenerativeAlgorithm*, AlgorithmCatalog::kAlgorithmsPerTrack>, TrackCount>& builtinTrackAlgorithms) const
{
    if (trackIndex >= TrackCount)
        return nullptr;

    if (algorithmId < AlgorithmCatalog::kAlgorithmsPerTrack)
        return builtinTrackAlgorithms[trackIndex][algorithmId];

    if (algorithmRegistry != nullptr)
    {
        uint16_t customIdx = algorithmId - AlgorithmPresetRegistry::kCustomAlgorithmIdBase;
        if (customIdx < customTrackAlgorithms[trackIndex].size())
            return customTrackAlgorithms[trackIndex][customIdx].get();
    }

    return builtinTrackAlgorithms[trackIndex][0];
}

void CommandProcessor::process()
{
    Command cmd;
    while (commandQueue.pop(cmd))
    {
        switch (cmd.type)
        {
            case Command::Type::SetAlgorithm:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    uint16_t clamped = static_cast<uint16_t>(std::min<uint16_t>(cmd.paramId, getMaxAlgorithmIdForTrack(cmd.trackIndex)));
                    uint16_t previous = appState.tracks[cmd.trackIndex].algorithmId.load(std::memory_order_relaxed);
                    appState.tracks[cmd.trackIndex].setAlgorithmId(clamped);

                    if (cmd.trackIndex > 0)
                    {
                        const bool manualOverride = appState.tracks[cmd.trackIndex].synthManualOverride.load(std::memory_order_relaxed);
                        if (!manualOverride)
                        {
                            const uint8_t defaultPreset = SynthCatalog::getDefaultPresetForAlgorithm(cmd.trackIndex, static_cast<uint8_t>(std::min<uint16_t>(clamped, 255)));
                            delegate.applySoundPreset(cmd.trackIndex, defaultPreset, false);
                        }
                    }

                    if (previous != clamped)
                        delegate.resetAlgorithm(cmd.trackIndex, clamped);
                }
                break;
            }
            case Command::Type::StepAlgorithm:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    const uint16_t maxAlgo = getMaxAlgorithmIdForTrack(cmd.trackIndex);
                    const uint16_t previous = appState.tracks[cmd.trackIndex].algorithmId.load(std::memory_order_relaxed);
                    int next = static_cast<int>(previous) + static_cast<int>(cmd.value);
                    if (next < 0)
                        next = static_cast<int>(maxAlgo);
                    else if (next > static_cast<int>(maxAlgo))
                        next = 0;

                    const uint16_t nextAlgo = static_cast<uint16_t>(next);
                    appState.tracks[cmd.trackIndex].setAlgorithmId(nextAlgo);

                    if (cmd.trackIndex > 0)
                    {
                        const bool manualOverride = appState.tracks[cmd.trackIndex].synthManualOverride.load(std::memory_order_relaxed);
                        if (!manualOverride)
                        {
                            const uint8_t defaultPreset = SynthCatalog::getDefaultPresetForAlgorithm(cmd.trackIndex, static_cast<uint8_t>(std::min<uint16_t>(nextAlgo, 255)));
                            delegate.applySoundPreset(cmd.trackIndex, defaultPreset, false);
                        }
                    }

                    if (previous != nextAlgo)
                        delegate.resetAlgorithm(cmd.trackIndex, nextAlgo);
                }
                break;
            }
            case Command::Type::SetSynthPreset:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    const uint8_t maxPreset = delegate.getMaxSynthPresetIdForTrack(cmd.trackIndex);
                    const uint8_t preset = static_cast<uint8_t>(std::min<uint16_t>(cmd.paramId, maxPreset));
                    delegate.applySoundPreset(cmd.trackIndex, preset, true);
                }
                break;
            }
            case Command::Type::StepSynthPreset:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    const uint8_t maxPreset = delegate.getMaxSynthPresetIdForTrack(cmd.trackIndex);
                    const uint8_t previousPreset = appState.tracks[cmd.trackIndex].synthPreset.load(std::memory_order_relaxed);
                    int nextPreset = static_cast<int>(previousPreset) + static_cast<int>(cmd.value);
                    if (nextPreset < 0)
                        nextPreset = maxPreset;
                    else if (nextPreset > maxPreset)
                        nextPreset = 0;

                    delegate.applySoundPreset(cmd.trackIndex, static_cast<uint8_t>(nextPreset), true);
                }
                break;
            }
            case Command::Type::SetDensity:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float current = appState.tracks[cmd.trackIndex].density.load(std::memory_order_relaxed);
                    float next = std::clamp(current + cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setDensity(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        appState.setArrangementSectionTrackParameter(appState.arrangementCurrentSection.load(std::memory_order_relaxed), cmd.trackIndex, 0, next);
                    }
                }
                break;
            }
            case Command::Type::SetComplexity:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float current = appState.tracks[cmd.trackIndex].complexity.load(std::memory_order_relaxed);
                    float next = std::clamp(current + cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setComplexity(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        appState.setArrangementSectionTrackParameter(appState.arrangementCurrentSection.load(std::memory_order_relaxed), cmd.trackIndex, 1, next);
                    }
                }
                break;
            }
            case Command::Type::SetTone:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float current = appState.tracks[cmd.trackIndex].tone.load(std::memory_order_relaxed);
                    float next = std::clamp(current + cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setTone(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        appState.setArrangementSectionTrackParameter(appState.arrangementCurrentSection.load(std::memory_order_relaxed), cmd.trackIndex, 2, next);
                    }
                }
                break;
            }
            case Command::Type::SetMotion:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float current = appState.tracks[cmd.trackIndex].motion.load(std::memory_order_relaxed);
                    float next = std::clamp(current + cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setMotion(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        appState.setArrangementSectionTrackParameter(appState.arrangementCurrentSection.load(std::memory_order_relaxed), cmd.trackIndex, 3, next);
                    }
                }
                break;
            }
            case Command::Type::SetTrackGain:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float current = appState.tracks[cmd.trackIndex].gain.load(std::memory_order_relaxed);
                    float next = std::clamp(current + cmd.value, 0.0f, AppState::kMaxTrackGain);
                    appState.tracks[cmd.trackIndex].setGain(next);
                }
                else if (cmd.trackIndex == TrackCount)
                {
                    float current = appState.master.gain.load(std::memory_order_relaxed);
                    float next = std::clamp(current + cmd.value, 0.0f, AppState::kMaxMasterGain);
                    appState.master.setGain(next);
                }
                break;
            }
            case Command::Type::SetChordProg:
            {
                const int progressionCount = ChordProgression::getNumProgressions();
                const uint16_t maxProgression = progressionCount > 0 ? static_cast<uint16_t>(progressionCount - 1) : 0;
                const uint16_t clamped = std::min<uint16_t>(cmd.paramId, maxProgression);
                appState.setChordProgression(static_cast<uint8_t>(clamped));
                break;
            }
            case Command::Type::SetGenre:
            {
                if (cmd.paramId <= GenreCatalog::kGenreCount)
                    appState.setGenre(static_cast<uint8_t>(cmd.paramId));
                break;
            }
            case Command::Type::RandomizeForGenre:
            {
                uint8_t genreId = static_cast<uint8_t>(cmd.paramId);
                if (genreId > GenreCatalog::kGenreCount) break;

                const bool freeRandomization = genreId == 0;
                const float minBpm = freeRandomization ? 80.0f : GenreCatalog::getGenreMinBpm(static_cast<uint8_t>(genreId - 1));
                const float maxBpm = freeRandomization ? 160.0f : GenreCatalog::getGenreMaxBpm(static_cast<uint8_t>(genreId - 1));
                const float range = maxBpm - minBpm;
                const float randomBpm = minBpm + range * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                appState.setBpm(randomBpm);
                appState.setGenre(genreId);
                appState.setProjectKey(static_cast<uint8_t>(rand() % 12),
                                       static_cast<uint8_t>(rand() % 2));

                const int progressionCount = ChordProgression::getNumProgressions();
                if (progressionCount > 0)
                {
                    uint8_t matchingProgressions[256];
                    int progressionMatchCount = 0;

                    for (int progression = 0; progression < progressionCount && progressionMatchCount < 256; ++progression)
                    {
                        const bool matches = freeRandomization
                            || ChordProgression::hasGenre(progression, genreId);
                        if (matches)
                            matchingProgressions[progressionMatchCount++] = static_cast<uint8_t>(progression);
                    }

                    if (progressionMatchCount > 0)
                        appState.setChordProgression(matchingProgressions[rand() % progressionMatchCount]);
                }

                for (uint8_t track = 0; track < AppState::kTrackCount; ++track)
                {
                    uint16_t matching[256];
                    int count = 0;

                    for (uint16_t algo = 0; algo < AlgorithmCatalog::kAlgorithmsPerTrack && count < 256; ++algo)
                    {
                        bool matches = freeRandomization;
                        if (!matches)
                            matches = GenreCatalog::algorithmHasGenre(track, algo, genreId);
                        if (matches)
                            matching[count++] = algo;
                    }

                    if (algorithmRegistry != nullptr && (freeRandomization || genreId > 0))
                    {
                        auto customPresets = algorithmRegistry->listForTrack(track);
                        for (const auto& preset : customPresets)
                        {
                            if (count >= 256) break;
                            bool matches = freeRandomization;
                            if (!matches)
                            {
                                auto runtimeId = algorithmRegistry->runtimeIdForPresetId(track, preset.id);
                                if (runtimeId.has_value() && ((preset.genreTags >> (genreId - 1)) & 1))
                                    matching[count++] = runtimeId.value();
                            }
                            else
                            {
                                auto runtimeId = algorithmRegistry->runtimeIdForPresetId(track, preset.id);
                                if (runtimeId.has_value())
                                    matching[count++] = runtimeId.value();
                            }
                        }
                    }

                    if (count > 0)
                    {
                        const uint16_t previous = appState.tracks[track].algorithmId.load(std::memory_order_relaxed);
                        const uint16_t selected = matching[rand() % count];
                        appState.tracks[track].setAlgorithmId(selected);
                        if (previous != selected)
                            delegate.resetAlgorithm(track, selected);
                    }

                    uint8_t matchingSounds[SynthCatalog::kMaxPresetsPerTrack];
                    int soundCount = 0;
                    const uint16_t presetCount = SynthCatalog::getAutoSelectablePresetCountForTrack(track);
                    for (uint16_t preset = 0; preset < presetCount && soundCount < SynthCatalog::kMaxPresetsPerTrack; ++preset)
                    {
                        const bool matches = freeRandomization
                            || SynthCatalog::soundHasGenre(track, static_cast<uint8_t>(preset), genreId);
                        if (matches)
                            matchingSounds[soundCount++] = static_cast<uint8_t>(preset);
                    }

                    if (soundCount > 0)
                        delegate.applySoundPreset(track, matchingSounds[rand() % soundCount], false);
                }
                break;
            }
            case Command::Type::SetArrangementSectionCount:
            {
                const uint8_t nextCount = static_cast<uint8_t>(std::clamp<uint16_t>(cmd.paramId, 1, AppState::kArrangementMaxSections));
                appState.setArrangementSectionCount(nextCount);
                if (appState.arrangementCurrentSection.load(std::memory_order_relaxed) >= nextCount)
                    appState.setArrangementCurrentSection(static_cast<uint8_t>(nextCount - 1));
                delegate.setArrangementAnchorInitialized(false);
                break;
            }
            case Command::Type::SetArrangementSection:
            {
                const uint8_t sectionCount = static_cast<uint8_t>(std::max<uint8_t>(appState.arrangementSectionCount.load(std::memory_order_relaxed), 1));
                const uint8_t nextSection = static_cast<uint8_t>(std::min<uint16_t>(cmd.paramId, sectionCount - 1));
                appState.setArrangementCurrentSection(nextSection);
                delegate.setArrangementAnchorInitialized(false);
                break;
            }
            case Command::Type::SetArrangementMode:
            {
                appState.setArrangementMode(static_cast<uint8_t>(std::min<uint16_t>(cmd.paramId, AppState::kArrangementModeMixed)));
                break;
            }
            case Command::Type::StepArrangementSection:
            {
                const int step = (cmd.value > 0.0f) ? 1 : ((cmd.value < 0.0f) ? -1 : 0);
                if (step != 0)
                {
                    const uint8_t sectionCount = static_cast<uint8_t>(std::max<uint8_t>(appState.arrangementSectionCount.load(std::memory_order_relaxed), 1));
                    const uint8_t current = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
                    int next = static_cast<int>(current) + step;
                    if (next < 0)
                        next = static_cast<int>(sectionCount - 1);
                    else if (next >= sectionCount)
                        next = 0;
                    appState.setArrangementCurrentSection(static_cast<uint8_t>(next));
                    delegate.setArrangementAnchorInitialized(false);
                }
                break;
            }
            case Command::Type::StepArrangementMode:
            {
                const uint8_t currentMode = appState.arrangementMode.load(std::memory_order_relaxed);
                const uint8_t nextMode = static_cast<uint8_t>((currentMode + 1u) % 3u);
                appState.setArrangementMode(nextMode);
                break;
            }
            case Command::Type::SetArrangementSectionLength:
            {
                const uint8_t sectionIndex = Command::decodeArrangementSectionIndex(cmd.paramId);
                const uint8_t bars = static_cast<uint8_t>(std::min<uint16_t>(Command::decodeArrangementValue(cmd.paramId), AppState::kArrangementMaxSectionLengthBars));
                appState.setArrangementSectionLength(sectionIndex, bars);
                break;
            }
            case Command::Type::SetArrangementSectionProgression:
            {
                const uint8_t sectionIndex = Command::decodeArrangementSectionIndex(cmd.paramId);
                const uint16_t rawValue = Command::decodeArrangementValue(cmd.paramId);
                const uint8_t progressionValue = static_cast<uint8_t>(std::min<uint16_t>(rawValue, 0xFFu));
                if (progressionValue == AppState::kArrangementProgressionFollowGlobal)
                {
                    appState.setArrangementSectionProgression(sectionIndex, progressionValue);
                    break;
                }
                const int progressionCount = ChordProgression::getNumProgressions();
                const uint8_t maxProgression = progressionCount > 0 ? static_cast<uint8_t>(progressionCount - 1) : 0;
                appState.setArrangementSectionProgression(sectionIndex, static_cast<uint8_t>(std::min<uint8_t>(progressionValue, maxProgression)));
                break;
            }
            case Command::Type::SetArrangementSectionTrackMask:
            {
                const uint8_t sectionIndex = Command::decodeArrangementSectionIndex(cmd.paramId);
                const uint8_t trackMask = static_cast<uint8_t>(Command::decodeArrangementValue(cmd.paramId) & AppState::kArrangementTrackMaskAll);
                appState.setArrangementSectionTrackMask(sectionIndex, trackMask);
                break;
            }
            case Command::Type::SetArrangementSectionParametersEnabled:
            {
                appState.setArrangementSectionParametersEnabled(cmd.paramId != 0);
                break;
            }
            case Command::Type::SetArrangementSectionTrackParameter:
            {
                const uint8_t encoded = cmd.trackIndex;
                const uint8_t trackIndex = static_cast<uint8_t>(encoded % AppState::kTrackCount);
                const uint8_t parameterIndex = static_cast<uint8_t>(encoded / AppState::kTrackCount);
                const uint8_t sectionIndex = static_cast<uint8_t>(std::min<uint16_t>(cmd.paramId, AppState::kArrangementMaxSections - 1));
                appState.setArrangementSectionTrackParameter(sectionIndex, trackIndex, parameterIndex, cmd.value);
                break;
            }
            case Command::Type::SetArrangementChainEnabled:
            {
                appState.setArrangementChainEnabled(cmd.paramId != 0);
                delegate.setArrangementAnchorInitialized(false);
                break;
            }
            case Command::Type::SetArrangementChainLength:
            {
                const uint8_t chainLength = static_cast<uint8_t>(std::clamp<uint16_t>(cmd.paramId, 1, AppState::kArrangementMaxSections));
                appState.setArrangementChainLength(chainLength);
                delegate.setArrangementAnchorInitialized(false);
                break;
            }
            case Command::Type::SetArrangementChainStep:
            {
                const uint8_t chainIndex = Command::decodeArrangementSectionIndex(cmd.paramId);
                const uint8_t sectionIndex = static_cast<uint8_t>(std::min<uint16_t>(Command::decodeArrangementValue(cmd.paramId), static_cast<uint16_t>(AppState::kArrangementMaxSections - 1)));
                appState.setArrangementChainStep(chainIndex, sectionIndex);
                delegate.setArrangementAnchorInitialized(false);
                break;
            }
            case Command::Type::SetProjectKey:
            {
                const uint8_t keyRoot = static_cast<uint8_t>(Command::decodeProjectKeyRoot(cmd.paramId) % 12);
                const uint8_t rawMode = Command::decodeProjectKeyMode(cmd.paramId);
                const uint8_t keyMode = (rawMode == AppState::kProjectKeyModeMajor) ? AppState::kProjectKeyModeMajor : AppState::kProjectKeyModeNaturalMinor;
                appState.setProjectKey(keyRoot, keyMode);
                break;
            }
            case Command::Type::SetTempo:
            {
                float currentBpm = appState.bpm.load(std::memory_order_relaxed);
                float newBpm = std::clamp(currentBpm + cmd.value, 20.0f, 260.0f);
                appState.setBpm(newBpm);
                break;
            }
            case Command::Type::RebuildCustomAlgorithms:
            {
                rebuildCustomAlgorithmInstances();
                break;
            }
            case Command::Type::SetDensityAbsolute:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float next = std::clamp(cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setDensity(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        const uint8_t section = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
                        appState.setArrangementSectionTrackParameter(section, cmd.trackIndex, 0, next);
                    }
                }
                break;
            }
            case Command::Type::SetComplexityAbsolute:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float next = std::clamp(cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setComplexity(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        const uint8_t section = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
                        appState.setArrangementSectionTrackParameter(section, cmd.trackIndex, 1, next);
                    }
                }
                break;
            }
            case Command::Type::SetToneAbsolute:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float next = std::clamp(cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setTone(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        const uint8_t section = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
                        appState.setArrangementSectionTrackParameter(section, cmd.trackIndex, 2, next);
                    }
                }
                break;
            }
            case Command::Type::SetMotionAbsolute:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float next = std::clamp(cmd.value, 0.0f, 1.0f);
                    appState.tracks[cmd.trackIndex].setMotion(next);
                    if (appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed))
                    {
                        const uint8_t section = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
                        appState.setArrangementSectionTrackParameter(section, cmd.trackIndex, 3, next);
                    }
                }
                break;
            }
            case Command::Type::SetTrackGainAbsolute:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    float next = std::clamp(cmd.value, 0.0f, AppState::kMaxTrackGain);
                    appState.tracks[cmd.trackIndex].setGain(next);
                }
                else if (cmd.trackIndex == TrackCount)
                {
                    float next = std::clamp(cmd.value, 0.0f, AppState::kMaxMasterGain);
                    appState.master.setGain(next);
                }
                break;
            }
            case Command::Type::SetTempoAbsolute:
            {
                float newBpm = std::clamp(cmd.value, 20.0f, 260.0f);
                appState.setBpm(newBpm);
                break;
            }
            case Command::Type::PlayStop:
            {
                bool playing = appState.playing.load(std::memory_order_relaxed);
                appState.setPlaying(!playing);
                break;
            }
            case Command::Type::Stop:
            {
                appState.setPlaying(false);
                delegate.resetTransportAndArrangement();
                break;
            }
            case Command::Type::ToggleMetronome:
            {
                bool clickEnabled = appState.metronomeEnabled.load(std::memory_order_relaxed);
                appState.setMetronomeEnabled(!clickEnabled);
                break;
            }
            case Command::Type::ToggleTrackMute:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    bool muted = appState.tracks[cmd.trackIndex].muted.load(std::memory_order_relaxed);
                    appState.tracks[cmd.trackIndex].setMuted(!muted);
                }
                break;
            }
            case Command::Type::SetTrackEffectPreset:
            {
                if (cmd.trackIndex < TrackCount)
                {
                    const uint8_t slotIndex = Command::decodeEffectSlotIndex(cmd.paramId);
                    const uint16_t presetId = Command::decodeEffectPresetId(cmd.paramId);
                    if (slotIndex < EffectProcessor::EffectSlotCount)
                        effectProcessor.applyTrackEffectPreset(cmd.trackIndex, slotIndex, presetId);
                }
                break;
            }
            case Command::Type::SetMasterEffectPreset:
            {
                const uint8_t slotIndex = Command::decodeEffectSlotIndex(cmd.paramId);
                const uint16_t presetId = Command::decodeEffectPresetId(cmd.paramId);
                if (slotIndex < EffectProcessor::EffectSlotCount)
                    effectProcessor.applyMasterEffectPreset(slotIndex, presetId);
                break;
            }
            case Command::Type::SpotEffectOn:
            {
                if (!SpotEffectCatalog::isValidSpotEffectId(cmd.paramId))
                    break;
                const uint8_t effectBit = SpotEffectCatalog::getBitMask(static_cast<Command::SpotEffectId>(cmd.paramId));
                if (effectBit != 0u)
                {
                    const uint8_t currentMask = appState.activeSpotEffects.load(std::memory_order_relaxed);
                    const uint8_t nextMask = static_cast<uint8_t>(currentMask | effectBit);
                    appState.setActiveSpotEffects(nextMask);
                    effectProcessor.applySpotEffectsBitmask(nextMask);
                }
                break;
            }
            case Command::Type::SpotEffectOff:
            {
                if (!SpotEffectCatalog::isValidSpotEffectId(cmd.paramId))
                    break;
                const uint8_t effectBit = SpotEffectCatalog::getBitMask(static_cast<Command::SpotEffectId>(cmd.paramId));
                if (effectBit != 0u)
                {
                    const uint8_t currentMask = appState.activeSpotEffects.load(std::memory_order_relaxed);
                    const uint8_t nextMask = static_cast<uint8_t>(currentMask & static_cast<uint8_t>(~effectBit));
                    appState.setActiveSpotEffects(nextMask);
                    effectProcessor.applySpotEffectsBitmask(nextMask);
                }
                break;
            }
            case Command::Type::SpotEffectToggle:
            {
                if (!SpotEffectCatalog::isValidSpotEffectId(cmd.paramId))
                    break;
                const uint8_t effectBit = SpotEffectCatalog::getBitMask(static_cast<Command::SpotEffectId>(cmd.paramId));
                if (effectBit != 0u)
                {
                    const uint8_t currentMask = appState.activeSpotEffects.load(std::memory_order_relaxed);
                    const uint8_t nextMask = static_cast<uint8_t>(currentMask ^ effectBit);
                    appState.setActiveSpotEffects(nextMask);
                    effectProcessor.applySpotEffectsBitmask(nextMask);
                }
                break;
            }
            case Command::Type::SetDrumSampleAssignment:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = Command::decodeDrumSlotIndex(cmd.paramId);
                    const uint16_t sampleId = Command::decodeDrumSampleId(cmd.paramId);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        appState.tracks[0].setDrumSampleSlotSampleId(slotIndex, sampleId);
                        const DrumSampleData* sampleData = drumSampleLibrary != nullptr ? drumSampleLibrary->getRtSample(sampleId) : nullptr;
                        delegate.setDrumSampleForSlot(slotIndex, sampleData);
                    }
                }
                break;
            }
            case Command::Type::ClearDrumSampleAssignment:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = static_cast<uint8_t>(cmd.paramId & 0xFFu);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        appState.tracks[0].setDrumSampleSlotSampleId(slotIndex, 0);
                        delegate.setDrumSampleForSlot(slotIndex, nullptr);
                    }
                }
                break;
            }
            case Command::Type::SetDrumSampleVolume:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = static_cast<uint8_t>(cmd.paramId & 0xFFu);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        const float value = std::clamp(cmd.value, 0.0f, 2.0f);
                        appState.tracks[0].setDrumSampleSlotVolume(slotIndex, value);
                        delegate.setDrumSampleSlotVolume(slotIndex, value);
                    }
                }
                break;
            }
            case Command::Type::SetDrumSampleTune:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = static_cast<uint8_t>(cmd.paramId & 0xFFu);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        const float value = std::clamp(cmd.value, -24.0f, 24.0f);
                        appState.tracks[0].setDrumSampleSlotTuneSemitones(slotIndex, value);
                        delegate.setDrumSampleSlotTuneSemitones(slotIndex, value);
                    }
                }
                break;
            }
            case Command::Type::SetDrumSampleStartOffset:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = static_cast<uint8_t>(cmd.paramId & 0xFFu);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        const float value = std::clamp(cmd.value, 0.0f, 0.95f);
                        appState.tracks[0].setDrumSampleSlotStartOffset(slotIndex, value);
                        delegate.setDrumSampleSlotStartOffset(slotIndex, value);
                    }
                }
                break;
            }
            case Command::Type::SetDrumSampleDecay:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = static_cast<uint8_t>(cmd.paramId & 0xFFu);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        const float value = std::clamp(cmd.value, 0.0f, 1.0f);
                        appState.tracks[0].setDrumSampleSlotDecay(slotIndex, value);
                        delegate.setDrumSampleSlotDecay(slotIndex, value);
                    }
                }
                break;
            }
            case Command::Type::SetDrumSampleVelocitySensitivity:
            {
                if (cmd.trackIndex == 0)
                {
                    const uint8_t slotIndex = static_cast<uint8_t>(cmd.paramId & 0xFFu);
                    if (slotIndex < AppState::TrackState::DrumSampleSlotCount)
                    {
                        const float value = std::clamp(cmd.value, 0.0f, 1.0f);
                        appState.tracks[0].setDrumSampleSlotVelocitySensitivity(slotIndex, value);
                        delegate.setDrumSampleSlotVelocitySensitivity(slotIndex, value);
                    }
                }
                break;
            }
        }
    }
}
