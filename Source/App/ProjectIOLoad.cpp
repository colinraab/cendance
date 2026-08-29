#include "ProjectIOLoad.h"

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


constexpr const char* kProjectFormat = "cendanceProject";

bool readStringProperty(const juce::DynamicObject* object,
                        const char* key,
                        std::string& value) {
    if (object == nullptr || !object->hasProperty(key)) {
        return false;
    }

    value = object->getProperty(key).toString().toStdString();
    return true;
}

bool readBoolProperty(const juce::DynamicObject* object,
                      const char* key,
                      bool& value) {
    if (object == nullptr || !object->hasProperty(key)) {
        return false;
    }

    value = static_cast<bool>(object->getProperty(key));
    return true;
}

bool readUIntProperty(const juce::DynamicObject* object,
                      const char* key,
                      uint32_t& value) {
    if (object == nullptr || !object->hasProperty(key)) {
        return false;
    }

    const auto raw = static_cast<int64_t>(object->getProperty(key));
    if (raw < 0 || raw > static_cast<int64_t>(std::numeric_limits<uint32_t>::max())) {
        return false;
    }

    value = static_cast<uint32_t>(raw);
    return true;
}

bool readFloatProperty(const juce::DynamicObject* object,
                       const char* key,
                       float& value) {
    if (object == nullptr || !object->hasProperty(key)) {
        return false;
    }

    value = static_cast<float>(static_cast<double>(object->getProperty(key)));
    return std::isfinite(value);
}

bool migrateLoadedSnapshotToCurrent(ProjectSnapshot& snapshot,
                                    uint32_t sourceSchemaMajor,
                                    uint32_t sourceSchemaMinor,
                                    std::string& error) {
    error.clear();

    switch (sourceSchemaMajor) {
        case 1:
            snapshot.schemaMajor = kProjectSchemaMajor;
            snapshot.schemaMinor = kProjectSchemaMinor;
            return true;
        case kProjectForwardCompatibleSchemaMajor:
            // v2 migration scaffold: keep reading the shared core fields and
            // normalize to v1 runtime representation until dedicated v2 fields are added.
            snapshot.schemaMajor = kProjectSchemaMajor;
            snapshot.schemaMinor = kProjectSchemaMinor;
            juce::ignoreUnused(sourceSchemaMinor);
            return true;
        default:
            error = "Unsupported project schema major version.";
            return false;
    }
}


} // namespace

bool saveProjectFile(const ProjectSnapshot& snapshot,
                     const std::string& path,
                     std::string& error) {
    error.clear();

    std::string validationError;
    if (!validateSnapshot(snapshot, validationError)) {
        error = validationError;
        return false;
    }

    std::filesystem::path filePath(path);
    try {
        if (filePath.has_parent_path()) {
            std::filesystem::create_directories(filePath.parent_path());
        }
    } catch (const std::exception& ex) {
        error = std::string("Failed to create project directory: ") + ex.what();
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("format", kProjectFormat);
    root->setProperty("projectName", juce::String(snapshot.projectName));
    root->setProperty("savedAtUtc", juce::String(snapshot.savedAtUtc));

    auto schemaObject = std::make_unique<juce::DynamicObject>();
    schemaObject->setProperty("major", static_cast<int>(snapshot.schemaMajor));
    schemaObject->setProperty("minor", static_cast<int>(snapshot.schemaMinor));
    root->setProperty("schemaVersion", juce::var(schemaObject.release()));

    auto stateObject = std::make_unique<juce::DynamicObject>();
    stateObject->setProperty("bpm", snapshot.bpm);
    stateObject->setProperty("playing", snapshot.playing);
    stateObject->setProperty("metronomeEnabled", snapshot.metronomeEnabled);
    stateObject->setProperty("chordProgression", static_cast<int>(snapshot.chordProgression));
    stateObject->setProperty("projectKeyRoot", static_cast<int>(snapshot.projectKeyRoot));
    stateObject->setProperty("projectKeyMode", static_cast<int>(snapshot.projectKeyMode));

    auto arrangementObject = std::make_unique<juce::DynamicObject>();
    arrangementObject->setProperty("sectionCount", static_cast<int>(snapshot.arrangementSectionCount));
    arrangementObject->setProperty("currentSection", static_cast<int>(snapshot.arrangementCurrentSection));
    arrangementObject->setProperty("mode", static_cast<int>(snapshot.arrangementMode));
    arrangementObject->setProperty("chainEnabled", snapshot.arrangementChainEnabled);
    arrangementObject->setProperty("chainLength", static_cast<int>(snapshot.arrangementChainLength));
    arrangementObject->setProperty("sectionParametersEnabled", snapshot.arrangementSectionParametersEnabled);

    juce::Array<juce::var> sectionLengths;
    juce::Array<juce::var> sectionProgressions;
    juce::Array<juce::var> sectionTrackMasks;
    juce::Array<juce::var> chainSequence;
    for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
        sectionLengths.add(static_cast<int>(snapshot.arrangementSectionLengths[section]));
        sectionProgressions.add(static_cast<int>(snapshot.arrangementSectionProgressions[section]));
        sectionTrackMasks.add(static_cast<int>(snapshot.arrangementSectionTrackMasks[section]));
        chainSequence.add(static_cast<int>(snapshot.arrangementChainSequence[section]));
    }
    juce::Array<juce::var> sectionTrackParameters;
    for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
        juce::Array<juce::var> sectionArray;
        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
            juce::Array<juce::var> trackArray;
            for (uint8_t parameter = 0; parameter < AppState::kArrangementTrackParameterCount; ++parameter) {
                trackArray.add(snapshot.arrangementSectionTrackParameters[section][track][parameter]);
            }
            sectionArray.add(juce::var(trackArray));
        }
        sectionTrackParameters.add(juce::var(sectionArray));
    }

    arrangementObject->setProperty("sectionLengths", juce::var(sectionLengths));
    arrangementObject->setProperty("sectionProgressions", juce::var(sectionProgressions));
    arrangementObject->setProperty("sectionTrackMasks", juce::var(sectionTrackMasks));
    arrangementObject->setProperty("chainSequence", juce::var(chainSequence));
    arrangementObject->setProperty("sectionTrackParameters", juce::var(sectionTrackParameters));
    stateObject->setProperty("arrangement", juce::var(arrangementObject.release()));

    juce::Array<juce::var> tracksArray;
    for (uint8_t track = 0; track < kProjectTrackCount; ++track) {
        const auto& trackSnapshot = snapshot.tracks[track];
        auto trackObject = std::make_unique<juce::DynamicObject>();
        trackObject->setProperty("algorithmId", static_cast<int>(trackSnapshot.algorithmId));
        if (!trackSnapshot.customAlgorithmRef.empty())
            trackObject->setProperty("customAlgorithmRef", juce::String(trackSnapshot.customAlgorithmRef));
        trackObject->setProperty("synthPreset", static_cast<int>(trackSnapshot.synthPreset));
        trackObject->setProperty("soundPresetRef", juce::String(trackSnapshot.soundPresetRef));
        trackObject->setProperty("synthManualOverride", trackSnapshot.synthManualOverride);
        trackObject->setProperty("density", trackSnapshot.density);
        trackObject->setProperty("complexity", trackSnapshot.complexity);
        trackObject->setProperty("tone", trackSnapshot.tone);
        trackObject->setProperty("motion", trackSnapshot.motion);
        trackObject->setProperty("gain", trackSnapshot.gain);
        trackObject->setProperty("muted", trackSnapshot.muted);

        juce::Array<juce::var> effectSlots;
        juce::Array<juce::var> effectRefs;
        for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
            effectSlots.add(static_cast<int>(trackSnapshot.effectPresetSlots[slot]));
            effectRefs.add(juce::String(trackSnapshot.effectPresetRefs[slot]));
        }
        trackObject->setProperty("effectPresetSlots", juce::var(effectSlots));
        trackObject->setProperty("effectPresetRefs", juce::var(effectRefs));

        juce::Array<juce::var> drumSampleSlots;
        for (uint8_t slot = 0; slot < kProjectDrumSampleSlotCount; ++slot) {
            const auto& drumSlot = trackSnapshot.drumSampleSlots[slot];
            auto drumSlotObject = std::make_unique<juce::DynamicObject>();
            drumSlotObject->setProperty("sampleId", static_cast<int>(drumSlot.sampleId));
            drumSlotObject->setProperty("volume", drumSlot.volume);
            drumSlotObject->setProperty("tuneSemitones", drumSlot.tuneSemitones);
            drumSlotObject->setProperty("startOffset", drumSlot.startOffset);
            drumSlotObject->setProperty("decay", drumSlot.decay);
            drumSlotObject->setProperty("velocitySensitivity", drumSlot.velocitySensitivity);
            drumSampleSlots.add(juce::var(drumSlotObject.release()));
        }
        trackObject->setProperty("drumSampleSlots", juce::var(drumSampleSlots));

        tracksArray.add(juce::var(trackObject.release()));
    }

    stateObject->setProperty("tracks", juce::var(tracksArray));

    juce::Array<juce::var> masterSlots;
    juce::Array<juce::var> masterRefs;
    for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
        masterSlots.add(static_cast<int>(snapshot.masterEffectPresetSlots[slot]));
        masterRefs.add(juce::String(snapshot.masterEffectPresetRefs[slot]));
    }
    stateObject->setProperty("masterEffectPresetSlots", juce::var(masterSlots));
    stateObject->setProperty("masterEffectPresetRefs", juce::var(masterRefs));
    stateObject->setProperty("masterGain", snapshot.masterGain);

    root->setProperty("state", juce::var(stateObject.release()));

    const juce::String json = juce::JSON::toString(juce::var(root.release()), true);

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        error = "Unable to open project file for writing.";
        return false;
    }

    output << json.toStdString();
    if (!output.good()) {
        error = "Failed while writing project file.";
        return false;
    }

    return true;
}

bool loadProjectFile(const std::string& path,
                     ProjectSnapshot& outSnapshot,
                     std::string& error) {
    error.clear();

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        error = "Unable to open project file.";
        return false;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    if (!input.good() && !input.eof()) {
        error = "Unable to read project file.";
        return false;
    }

    juce::var parsed;
    const juce::Result parseResult = juce::JSON::parse(buffer.str(), parsed);
    if (parseResult.failed()) {
        error = "Project JSON parse failed: " + parseResult.getErrorMessage().toStdString();
        return false;
    }

    auto* root = parsed.getDynamicObject();
    if (root == nullptr) {
        error = "Project root must be an object.";
        return false;
    }

    const std::string format = root->getProperty("format").toString().toStdString();
    if (format != kProjectFormat) {
        error = "File is not a cendance project.";
        return false;
    }

    ProjectSnapshot snapshot;
    uint32_t sourceSchemaMajor = kProjectSchemaMajor;
    uint32_t sourceSchemaMinor = kProjectSchemaMinor;

    if (auto schema = root->getProperty("schemaVersion").getDynamicObject()) {
        uint32_t major = kProjectSchemaMajor;
        uint32_t minor = 0;
        if (!readUIntProperty(schema, "major", major)) {
            error = "schemaVersion.major is missing or invalid.";
            return false;
        }
        if (schema->hasProperty("minor") && !readUIntProperty(schema, "minor", minor)) {
            error = "schemaVersion.minor is invalid.";
            return false;
        }
        sourceSchemaMajor = major;
        sourceSchemaMinor = minor;
    }

    readStringProperty(root, "projectName", snapshot.projectName);
    readStringProperty(root, "savedAtUtc", snapshot.savedAtUtc);

    auto* state = root->getProperty("state").getDynamicObject();
    if (state == nullptr) {
        error = "state object is required.";
        return false;
    }

    if (!readFloatProperty(state, "bpm", snapshot.bpm)) {
        error = "state.bpm is missing or invalid.";
        return false;
    }
    readBoolProperty(state, "playing", snapshot.playing);
    if (!readBoolProperty(state, "metronomeEnabled", snapshot.metronomeEnabled)) {
        snapshot.metronomeEnabled = true;
    }

    uint32_t chordProgression = 0;
    if (!readUIntProperty(state, "chordProgression", chordProgression)) {
        error = "state.chordProgression is missing or invalid.";
        return false;
    }
    snapshot.chordProgression = static_cast<uint8_t>(std::min<uint32_t>(chordProgression, 255u));

    uint32_t projectKeyRoot = 0;
    if (readUIntProperty(state, "projectKeyRoot", projectKeyRoot)) {
        snapshot.projectKeyRoot = static_cast<uint8_t>(std::min<uint32_t>(projectKeyRoot, 255u));
    }

    uint32_t projectKeyMode = AppState::kProjectKeyModeNaturalMinor;
    if (readUIntProperty(state, "projectKeyMode", projectKeyMode)) {
        snapshot.projectKeyMode = static_cast<uint8_t>(std::min<uint32_t>(projectKeyMode, 255u));
    }

    if (auto* arrangement = state->getProperty("arrangement").getDynamicObject()) {
        uint32_t sectionCount = snapshot.arrangementSectionCount;
        uint32_t currentSection = snapshot.arrangementCurrentSection;
        uint32_t mode = snapshot.arrangementMode;
        bool chainEnabled = snapshot.arrangementChainEnabled;
        uint32_t chainLength = snapshot.arrangementChainLength;

        if (arrangement->hasProperty("sectionCount")
            && !readUIntProperty(arrangement, "sectionCount", sectionCount)) {
            error = "state.arrangement.sectionCount is invalid.";
            return false;
        }

        if (arrangement->hasProperty("currentSection")
            && !readUIntProperty(arrangement, "currentSection", currentSection)) {
            error = "state.arrangement.currentSection is invalid.";
            return false;
        }

        if (arrangement->hasProperty("mode") && !readUIntProperty(arrangement, "mode", mode)) {
            error = "state.arrangement.mode is invalid.";
            return false;
        }

        if (arrangement->hasProperty("chainEnabled")
            && !readBoolProperty(arrangement, "chainEnabled", chainEnabled)) {
            error = "state.arrangement.chainEnabled is invalid.";
            return false;
        }

        if (arrangement->hasProperty("chainLength")
            && !readUIntProperty(arrangement, "chainLength", chainLength)) {
            error = "state.arrangement.chainLength is invalid.";
            return false;
        }

        if (arrangement->hasProperty("sectionParametersEnabled")
            && !readBoolProperty(arrangement, "sectionParametersEnabled", snapshot.arrangementSectionParametersEnabled)) {
            error = "state.arrangement.sectionParametersEnabled is invalid.";
            return false;
        }

        snapshot.arrangementSectionCount = static_cast<uint8_t>(std::clamp<uint32_t>(
            sectionCount,
            1u,
            static_cast<uint32_t>(kProjectArrangementMaxSections)));
        snapshot.arrangementCurrentSection = static_cast<uint8_t>(std::min<uint32_t>(
            currentSection,
            static_cast<uint32_t>(snapshot.arrangementSectionCount - 1)));
        snapshot.arrangementMode = static_cast<uint8_t>(std::min<uint32_t>(mode,
                                                                           AppState::kArrangementModeMixed));
        snapshot.arrangementChainEnabled = chainEnabled;
        snapshot.arrangementChainLength = static_cast<uint8_t>(std::clamp<uint32_t>(
            chainLength,
            1u,
            static_cast<uint32_t>(kProjectArrangementMaxSections)));

        auto* sectionLengths = arrangement->getProperty("sectionLengths").getArray();
        if (sectionLengths != nullptr) {
            if (sectionLengths->size() != static_cast<int>(kProjectArrangementMaxSections)) {
                error = "state.arrangement.sectionLengths must have 8 entries when provided.";
                return false;
            }

            for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
                const auto raw = static_cast<int64_t>((*sectionLengths)[section]);
                if (raw < 0 || raw > std::numeric_limits<uint8_t>::max()) {
                    error = "state.arrangement.sectionLengths entry is invalid.";
                    return false;
                }
                snapshot.arrangementSectionLengths[section] = static_cast<uint8_t>(raw);
            }
        }

        auto* sectionProgressions = arrangement->getProperty("sectionProgressions").getArray();
        if (sectionProgressions != nullptr) {
            if (sectionProgressions->size() != static_cast<int>(kProjectArrangementMaxSections)) {
                error = "state.arrangement.sectionProgressions must have 8 entries when provided.";
                return false;
            }

            for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
                const auto raw = static_cast<int64_t>((*sectionProgressions)[section]);
                if (raw < 0 || raw > std::numeric_limits<uint8_t>::max()) {
                    error = "state.arrangement.sectionProgressions entry is invalid.";
                    return false;
                }
                snapshot.arrangementSectionProgressions[section] = static_cast<uint8_t>(raw);
            }
        }

        auto* sectionTrackMasks = arrangement->getProperty("sectionTrackMasks").getArray();
        if (sectionTrackMasks != nullptr) {
            if (sectionTrackMasks->size() != static_cast<int>(kProjectArrangementMaxSections)) {
                error = "state.arrangement.sectionTrackMasks must have 8 entries when provided.";
                return false;
            }

            for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
                const auto raw = static_cast<int64_t>((*sectionTrackMasks)[section]);
                if (raw < 0 || raw > std::numeric_limits<uint8_t>::max()) {
                    error = "state.arrangement.sectionTrackMasks entry is invalid.";
                    return false;
                }
                snapshot.arrangementSectionTrackMasks[section] = static_cast<uint8_t>(raw);
            }
        }

        auto* chainSequence = arrangement->getProperty("chainSequence").getArray();
        if (chainSequence != nullptr) {
            if (chainSequence->size() != static_cast<int>(kProjectArrangementMaxSections)) {
                error = "state.arrangement.chainSequence must have 8 entries when provided.";
                return false;
            }

            for (uint8_t chainStep = 0; chainStep < kProjectArrangementMaxSections; ++chainStep) {
                const auto raw = static_cast<int64_t>((*chainSequence)[chainStep]);
                if (raw < 0 || raw > std::numeric_limits<uint8_t>::max()) {
                    error = "state.arrangement.chainSequence entry is invalid.";
                    return false;
                }
                snapshot.arrangementChainSequence[chainStep] = static_cast<uint8_t>(raw);
            }
        }

        auto* sectionTrackParameters = arrangement->getProperty("sectionTrackParameters").getArray();
        if (sectionTrackParameters != nullptr) {
            if (sectionTrackParameters->size() != static_cast<int>(kProjectArrangementMaxSections)) {
                error = "state.arrangement.sectionTrackParameters must have 8 section entries when provided.";
                return false;
            }

            for (uint8_t section = 0; section < kProjectArrangementMaxSections; ++section) {
                auto* sectionArray = (*sectionTrackParameters)[section].getArray();
                if (sectionArray == nullptr || sectionArray->size() != static_cast<int>(AppState::kTrackCount)) {
                    error = "state.arrangement.sectionTrackParameters section entry is invalid.";
                    return false;
                }

                for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
                    auto* trackArray = (*sectionArray)[track].getArray();
                    if (trackArray == nullptr || trackArray->size() != static_cast<int>(AppState::kArrangementTrackParameterCount)) {
                        error = "state.arrangement.sectionTrackParameters track entry is invalid.";
                        return false;
                    }

                    for (uint8_t parameter = 0; parameter < AppState::kArrangementTrackParameterCount; ++parameter) {
                        const auto raw = static_cast<double>((*trackArray)[parameter]);
                        if (!std::isfinite(raw) || raw < 0.0 || raw > 1.0) {
                            error = "state.arrangement.sectionTrackParameters value is invalid.";
                            return false;
                        }
                        snapshot.arrangementSectionTrackParameters[section][track][parameter] = static_cast<float>(raw);
                    }
                }
            }
        }
    }

    // Backward-compat: legacy v1 files may include activeEffects. Spot trigger
    // state is runtime-only and intentionally ignored during load.
    uint32_t legacyActiveEffects = 0;
    juce::ignoreUnused(legacyActiveEffects);
    readUIntProperty(state, "activeEffects", legacyActiveEffects);

    auto* tracksArray = state->getProperty("tracks").getArray();
    if (tracksArray == nullptr || tracksArray->size() != static_cast<int>(kProjectTrackCount)) {
        error = "state.tracks must be an array of 4 tracks.";
        return false;
    }

    for (uint8_t track = 0; track < kProjectTrackCount; ++track) {
        auto* trackObject = (*tracksArray)[track].getDynamicObject();
        if (trackObject == nullptr) {
            error = "Track entry is not an object.";
            return false;
        }

        auto& trackSnapshot = snapshot.tracks[track];

        uint32_t algorithmId = 0;
        uint32_t synthPreset = 0;

        if (!readUIntProperty(trackObject, "algorithmId", algorithmId)) {
            error = "track.algorithmId is missing or invalid.";
            return false;
        }
        if (!readUIntProperty(trackObject, "synthPreset", synthPreset)) {
            error = "track.synthPreset is missing or invalid.";
            return false;
        }

        trackSnapshot.algorithmId = static_cast<uint16_t>(std::min<uint32_t>(algorithmId, 65535u));
        {
            juce::String customRef = trackObject->getProperty("customAlgorithmRef").toString();
            trackSnapshot.customAlgorithmRef = customRef.toStdString();
        }
        trackSnapshot.synthPreset = static_cast<uint8_t>(std::min<uint32_t>(synthPreset, 255u));
        trackSnapshot.soundPresetRef = trackObject->getProperty("soundPresetRef").toString().toStdString();
        readBoolProperty(trackObject, "synthManualOverride", trackSnapshot.synthManualOverride);

        if (!readFloatProperty(trackObject, "density", trackSnapshot.density)
            || !readFloatProperty(trackObject, "complexity", trackSnapshot.complexity)
            || !readFloatProperty(trackObject, "tone", trackSnapshot.tone)
            || !readFloatProperty(trackObject, "motion", trackSnapshot.motion)
            || !readFloatProperty(trackObject, "gain", trackSnapshot.gain)) {
            error = "track parameter fields are missing or invalid.";
            return false;
        }
        readBoolProperty(trackObject, "muted", trackSnapshot.muted);

        auto* effectSlots = trackObject->getProperty("effectPresetSlots").getArray();
        if (effectSlots == nullptr || effectSlots->size() != static_cast<int>(kProjectEffectSlotCount)) {
            error = "track.effectPresetSlots must have 3 entries.";
            return false;
        }

        for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
            const auto raw = static_cast<int64_t>((*effectSlots)[slot]);
            if (raw < 0 || raw > std::numeric_limits<uint16_t>::max()) {
                error = "track effect preset value is invalid.";
                return false;
            }
            trackSnapshot.effectPresetSlots[slot] = static_cast<uint16_t>(raw);
        }
        if (auto* effectRefs = trackObject->getProperty("effectPresetRefs").getArray();
            effectRefs != nullptr && effectRefs->size() == static_cast<int>(kProjectEffectSlotCount)) {
            for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
                trackSnapshot.effectPresetRefs[slot] = (*effectRefs)[slot].toString().toStdString();
            }
        }

        auto* drumSampleSlots = trackObject->getProperty("drumSampleSlots").getArray();
        if (drumSampleSlots != nullptr) {
            if (drumSampleSlots->size() != static_cast<int>(kProjectDrumSampleSlotCount)) {
                error = "track.drumSampleSlots must have 4 entries when provided.";
                return false;
            }

            for (uint8_t slot = 0; slot < kProjectDrumSampleSlotCount; ++slot) {
                auto* drumSlotObject = (*drumSampleSlots)[slot].getDynamicObject();
                if (drumSlotObject == nullptr) {
                    error = "track.drumSampleSlots entry must be an object.";
                    return false;
                }

                auto& drumSlot = trackSnapshot.drumSampleSlots[slot];
                uint32_t sampleId = 0;
                if (!readUIntProperty(drumSlotObject, "sampleId", sampleId)) {
                    error = "track.drumSampleSlots.sampleId is missing or invalid.";
                    return false;
                }

                if (!readFloatProperty(drumSlotObject, "volume", drumSlot.volume)
                    || !readFloatProperty(drumSlotObject, "tuneSemitones", drumSlot.tuneSemitones)
                    || !readFloatProperty(drumSlotObject, "startOffset", drumSlot.startOffset)
                    || !readFloatProperty(drumSlotObject, "decay", drumSlot.decay)
                    || !readFloatProperty(drumSlotObject, "velocitySensitivity", drumSlot.velocitySensitivity)) {
                    error = "track.drumSampleSlots parameter fields are missing or invalid.";
                    return false;
                }

                drumSlot.sampleId = static_cast<uint16_t>(std::min<uint32_t>(sampleId, std::numeric_limits<uint16_t>::max()));
            }
        }
    }

    auto* masterSlots = state->getProperty("masterEffectPresetSlots").getArray();
    if (masterSlots == nullptr || masterSlots->size() != static_cast<int>(kProjectEffectSlotCount)) {
        error = "state.masterEffectPresetSlots must have 3 entries.";
        return false;
    }

    for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
        const auto raw = static_cast<int64_t>((*masterSlots)[slot]);
        if (raw < 0 || raw > std::numeric_limits<uint16_t>::max()) {
            error = "master effect preset value is invalid.";
            return false;
        }
        snapshot.masterEffectPresetSlots[slot] = static_cast<uint16_t>(raw);
    }
    if (auto* masterRefs = state->getProperty("masterEffectPresetRefs").getArray();
        masterRefs != nullptr && masterRefs->size() == static_cast<int>(kProjectEffectSlotCount)) {
        for (uint8_t slot = 0; slot < kProjectEffectSlotCount; ++slot) {
            snapshot.masterEffectPresetRefs[slot] = (*masterRefs)[slot].toString().toStdString();
        }
    }
    if (state->hasProperty("masterGain") && !readFloatProperty(state, "masterGain", snapshot.masterGain)) {
        return false;
    }

    if (!migrateLoadedSnapshotToCurrent(snapshot, sourceSchemaMajor, sourceSchemaMinor, error)) {
        return false;
    }

    std::string validationError;
    if (!validateSnapshot(snapshot, validationError)) {
        error = validationError;
        return false;
    }

    outSnapshot = snapshot;
    return true;
}

} // namespace ProjectIO
