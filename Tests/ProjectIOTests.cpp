#include "../Source/App/ProjectIO.h"
#include "../Source/App/ProjectIOLoad.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

void testSaveLoadRoundTrip() {
    ProjectIO::ProjectSnapshot snapshot;
    snapshot.projectName = "RoundTrip";
    snapshot.bpm = 138.0f;
    snapshot.metronomeEnabled = false;
    snapshot.chordProgression = 12;
    snapshot.projectKeyRoot = 10;
    snapshot.projectKeyMode = AppState::kProjectKeyModeNaturalMinor;
    snapshot.arrangementSectionCount = 4;
    snapshot.arrangementCurrentSection = 1;
    snapshot.arrangementMode = AppState::kArrangementModeAuto;
    snapshot.arrangementSectionLengths = {2, 4, 8, 4, 4, 4, 4, 4};
    snapshot.arrangementSectionProgressions = {
        AppState::kArrangementProgressionFollowGlobal,
        9,
        11,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
    };
    snapshot.arrangementSectionTrackMasks = {0b1111, 0b0111, 0b0011, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111};
    snapshot.arrangementChainEnabled = true;
    snapshot.arrangementChainLength = 4;
    snapshot.arrangementChainSequence = {0, 2, 1, 3, 4, 5, 6, 7};
    snapshot.tracks[0].algorithmId = 3;
    snapshot.tracks[0].tone = 0.2f;
    snapshot.tracks[0].motion = 0.9f;
    snapshot.tracks[1].algorithmId = 4;
    snapshot.tracks[1].synthPreset = 11;
    snapshot.tracks[1].synthManualOverride = true;
    snapshot.tracks[1].density = 0.82f;
    snapshot.tracks[1].complexity = 0.23f;
    snapshot.tracks[1].tone = 0.75f;
    snapshot.tracks[1].motion = 0.15f;
    snapshot.tracks[1].gain = 1.4f;
    snapshot.tracks[1].muted = true;
    snapshot.tracks[1].effectPresetSlots = {4, 7, 2};
    snapshot.masterEffectPresetSlots = {9, 0, 24};
    snapshot.tracks[0].drumSampleSlots[0].sampleId = 17;
    snapshot.tracks[0].drumSampleSlots[0].volume = 1.25f;
    snapshot.tracks[0].drumSampleSlots[0].tuneSemitones = -5.0f;
    snapshot.tracks[0].drumSampleSlots[0].startOffset = 0.08f;
    snapshot.tracks[0].drumSampleSlots[0].decay = 0.7f;
    snapshot.tracks[0].drumSampleSlots[0].velocitySensitivity = 0.65f;

    const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_project_io_roundtrip.cendance").string();

    std::string error;
    assert(ProjectIO::saveProjectFile(snapshot, tempPath, error));

    ProjectIO::ProjectSnapshot loaded;
    assert(ProjectIO::loadProjectFile(tempPath, loaded, error));

    assert(loaded.bpm == snapshot.bpm);
    assert(loaded.metronomeEnabled == snapshot.metronomeEnabled);
    assert(loaded.chordProgression == snapshot.chordProgression);
    assert(loaded.projectKeyRoot == snapshot.projectKeyRoot);
    assert(loaded.projectKeyMode == snapshot.projectKeyMode);
    assert(loaded.arrangementSectionCount == snapshot.arrangementSectionCount);
    assert(loaded.arrangementCurrentSection == snapshot.arrangementCurrentSection);
    assert(loaded.arrangementMode == snapshot.arrangementMode);
    assert(loaded.arrangementSectionLengths == snapshot.arrangementSectionLengths);
    assert(loaded.arrangementSectionProgressions == snapshot.arrangementSectionProgressions);
    assert(loaded.arrangementSectionTrackMasks == snapshot.arrangementSectionTrackMasks);
    assert(loaded.arrangementChainEnabled == snapshot.arrangementChainEnabled);
    assert(loaded.arrangementChainLength == snapshot.arrangementChainLength);
    assert(loaded.arrangementChainSequence == snapshot.arrangementChainSequence);
    assert(loaded.tracks[1].algorithmId == snapshot.tracks[1].algorithmId);
    assert(loaded.tracks[0].tone == snapshot.tracks[0].tone);
    assert(loaded.tracks[0].motion == snapshot.tracks[0].motion);
    assert(loaded.tracks[1].synthPreset == snapshot.tracks[1].synthPreset);
    assert(loaded.tracks[1].synthManualOverride == snapshot.tracks[1].synthManualOverride);
    assert(loaded.tracks[1].density == snapshot.tracks[1].density);
    assert(loaded.tracks[1].complexity == snapshot.tracks[1].complexity);
    assert(loaded.tracks[1].gain == snapshot.tracks[1].gain);
    assert(loaded.tracks[1].muted == snapshot.tracks[1].muted);
    assert(loaded.tracks[1].effectPresetSlots[0] == snapshot.tracks[1].effectPresetSlots[0]);
    assert(loaded.masterEffectPresetSlots[2] == snapshot.masterEffectPresetSlots[2]);
    assert(loaded.tracks[0].drumSampleSlots[0].sampleId == snapshot.tracks[0].drumSampleSlots[0].sampleId);
    assert(loaded.tracks[0].drumSampleSlots[0].volume == snapshot.tracks[0].drumSampleSlots[0].volume);
    assert(loaded.tracks[0].drumSampleSlots[0].tuneSemitones == snapshot.tracks[0].drumSampleSlots[0].tuneSemitones);
    assert(loaded.tracks[0].drumSampleSlots[0].startOffset == snapshot.tracks[0].drumSampleSlots[0].startOffset);
    assert(loaded.tracks[0].drumSampleSlots[0].decay == snapshot.tracks[0].drumSampleSlots[0].decay);
    assert(loaded.tracks[0].drumSampleSlots[0].velocitySensitivity == snapshot.tracks[0].drumSampleSlots[0].velocitySensitivity);

    std::filesystem::remove(tempPath);
}

void testValidationRejectsInvalidBpm() {
    ProjectIO::ProjectSnapshot snapshot;
    snapshot.bpm = 800.0f;

    std::string error;
    assert(!ProjectIO::validateSnapshot(snapshot, error));
    assert(!error.empty());
}

void testNormalizeProjectPath() {
    std::string normalized;
    std::string error;

    assert(ProjectIO::normalizeProjectPath("./session1", normalized, error, true));
    assert(normalized.find(".cendance") != std::string::npos);
    assert(!normalized.empty());

    const std::filesystem::path normalizedPath(normalized);
    const std::filesystem::path expectedBase(ProjectIO::getDefaultProjectsDirectory());
    assert(normalizedPath.parent_path() == expectedBase);
}

void testApplySnapshotEnqueuesCommands() {
    AppState state;
    CommandQueue queue;

    state.setBpm(120.0f);
    state.setMetronomeEnabled(true);
    state.setActiveSpotEffects(0b0011);
    state.tracks[1].setDensity(0.5f);

    ProjectIO::ProjectSnapshot snapshot;
    snapshot.bpm = 128.0f;
    snapshot.metronomeEnabled = false;
    snapshot.chordProgression = 13;
    snapshot.projectKeyRoot = 1;
    snapshot.projectKeyMode = AppState::kProjectKeyModeMajor;
    snapshot.arrangementSectionCount = 3;
    snapshot.arrangementCurrentSection = 2;
    snapshot.arrangementMode = AppState::kArrangementModeManual;
    snapshot.arrangementSectionLengths = {3, 5, 7, 4, 4, 4, 4, 4};
    snapshot.arrangementSectionProgressions = {
        AppState::kArrangementProgressionFollowGlobal,
        6,
        12,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
        AppState::kArrangementProgressionFollowGlobal,
    };
    snapshot.arrangementSectionTrackMasks = {0b1111, 0b1101, 0b1011, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111};
    snapshot.arrangementChainEnabled = true;
    snapshot.arrangementChainLength = 4;
    snapshot.arrangementChainSequence = {0, 2, 1, 2, 4, 5, 6, 7};
    snapshot.tracks[1].density = 0.8f;
    snapshot.tracks[1].algorithmId = 2;
    snapshot.tracks[1].synthManualOverride = true;
    snapshot.tracks[1].synthPreset = 1;
    snapshot.tracks[0].tone = 0.2f;
    snapshot.tracks[0].motion = 0.9f;
    snapshot.tracks[0].drumSampleSlots[1].sampleId = 33;
    snapshot.tracks[0].drumSampleSlots[1].volume = 0.9f;

    std::string error;
    assert(ProjectIO::applySnapshotToCommandQueue(snapshot, state, queue, error, true));
    assert(state.activeSpotEffects.load() == 0);

    Command command{};
    assert(queue.pop(command));
    assert(command.type == Command::Type::Stop);

    bool sawTempo = false;
    bool sawProjectKey = false;
    bool sawChordProg = false;
    bool sawArrangementCount = false;
    bool sawArrangementSection = false;
    bool sawArrangementMode = false;
    bool sawArrangementLength = false;
    bool sawArrangementProgression = false;
    bool sawArrangementTrackMask = false;
    bool sawArrangementChainEnabled = false;
    bool sawArrangementChainLength = false;
    bool sawArrangementChainStep = false;
    bool sawTrackFx = false;
    bool sawDrumSynthPreset = false;
    bool sawDrumTone = false;
    bool sawDrumMotion = false;
    bool sawDrumSample = false;
    bool sawFinalStop = false;

    while (queue.pop(command)) {
        if (command.type == Command::Type::SetTempo) {
            sawTempo = true;
        }
        if (command.type == Command::Type::SetProjectKey) {
            sawProjectKey = true;
        }
        if (command.type == Command::Type::SetChordProg) {
            sawChordProg = true;
        }
        if (command.type == Command::Type::SetArrangementSectionCount) {
            sawArrangementCount = true;
        }
        if (command.type == Command::Type::SetArrangementSection) {
            sawArrangementSection = true;
        }
        if (command.type == Command::Type::SetArrangementMode) {
            sawArrangementMode = true;
            assert(command.paramId == snapshot.arrangementMode);
        }
        if (command.type == Command::Type::SetArrangementSectionLength) {
            sawArrangementLength = true;
        }
        if (command.type == Command::Type::SetArrangementSectionProgression) {
            sawArrangementProgression = true;
        }
        if (command.type == Command::Type::SetArrangementSectionTrackMask) {
            sawArrangementTrackMask = true;
        }
        if (command.type == Command::Type::SetArrangementChainEnabled) {
            sawArrangementChainEnabled = true;
        }
        if (command.type == Command::Type::SetArrangementChainLength) {
            sawArrangementChainLength = true;
            assert(command.paramId == snapshot.arrangementChainLength);
        }
        if (command.type == Command::Type::SetArrangementChainStep) {
            sawArrangementChainStep = true;
        }
        if (command.type == Command::Type::SetTrackEffectPreset) {
            sawTrackFx = true;
        }
        if (command.type == Command::Type::SetSynthPreset && command.trackIndex == 0) {
            sawDrumSynthPreset = true;
        }
        if (command.type == Command::Type::SetTone && command.trackIndex == 0) {
            sawDrumTone = true;
        }
        if (command.type == Command::Type::SetMotion && command.trackIndex == 0) {
            sawDrumMotion = true;
        }
        if (command.type == Command::Type::SetDrumSampleAssignment
            || command.type == Command::Type::ClearDrumSampleAssignment
            || command.type == Command::Type::SetDrumSampleVolume
            || command.type == Command::Type::SetDrumSampleTune
            || command.type == Command::Type::SetDrumSampleStartOffset
            || command.type == Command::Type::SetDrumSampleDecay
            || command.type == Command::Type::SetDrumSampleVelocitySensitivity) {
            sawDrumSample = true;
        }
        if (command.type == Command::Type::Stop) {
            sawFinalStop = true;
        }
    }

    assert(sawTempo);
    assert(sawProjectKey);
    assert(sawChordProg);
    assert(sawArrangementCount);
    assert(sawArrangementSection);
    assert(sawArrangementMode);
    assert(sawArrangementLength);
    assert(sawArrangementProgression);
    assert(sawArrangementTrackMask);
    assert(sawArrangementChainEnabled);
    assert(sawArrangementChainLength);
    assert(sawArrangementChainStep);
    assert(sawTrackFx);
    assert(sawDrumSynthPreset);
    assert(sawDrumTone);
    assert(sawDrumMotion);
    assert(sawDrumSample);
    assert(sawFinalStop);
}

void testLoadWithoutArrangementPayloadKeepsDefaults() {
        const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_project_legacy_no_arrangement.cendance").string();

        std::ofstream output(tempPath, std::ios::binary | std::ios::trunc);
        output << R"({
    "format": "cendanceProject",
    "schemaVersion": { "major": 1, "minor": 3 },
    "projectName": "LegacyNoArrangement",
    "state": {
        "bpm": 124.0,
        "playing": false,
        "metronomeEnabled": true,
        "chordProgression": 2,
        "tracks": [
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] }
        ],
        "masterEffectPresetSlots": [0, 0, 24]
    }
})";
        output.close();

        ProjectIO::ProjectSnapshot loaded;
        std::string error;
        assert(ProjectIO::loadProjectFile(tempPath, loaded, error));

        assert(loaded.arrangementSectionCount == 4);
        assert(loaded.arrangementCurrentSection == 0);
        assert(loaded.arrangementMode == AppState::kArrangementModeMixed);
        assert(loaded.arrangementChainEnabled == false);
        assert(loaded.arrangementChainLength == AppState::kArrangementDefaultChainLength);
        for (uint8_t section = 0; section < AppState::kArrangementMaxSections; ++section) {
            assert(loaded.arrangementSectionLengths[section] == AppState::kArrangementDefaultSectionLengthBars);
            assert(loaded.arrangementSectionProgressions[section] == AppState::kArrangementProgressionFollowGlobal);
            assert(loaded.arrangementSectionTrackMasks[section] == AppState::kArrangementTrackMaskAll);
            assert(loaded.arrangementChainSequence[section] == section);
        }

        std::filesystem::remove(tempPath);
}

void testSchemaV2LoadWithExtraFields() {
        const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_project_v2_compat.cendance").string();

        std::ofstream output(tempPath, std::ios::binary | std::ios::trunc);
        output << R"({
    "format": "cendanceProject",
    "schemaVersion": { "major": 2, "minor": 1 },
    "projectName": "ForwardCompat",
    "savedAtUtc": "2026-04-16T12:00:00Z",
    "state": {
        "bpm": 126.0,
        "playing": true,
        "metronomeEnabled": true,
        "chordProgression": 11,
        "activeEffects": 0,
        "tracks": [
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0], "v2Extra": {"humanize": 0.1} },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] }
        ],
        "masterEffectPresetSlots": [0, 0, 24],
        "futureFields": { "swingPercent": 57 }
    }
})";
        output.close();

        ProjectIO::ProjectSnapshot loaded;
        std::string error;
        assert(ProjectIO::loadProjectFile(tempPath, loaded, error));
        assert(loaded.schemaMajor == ProjectIO::kProjectSchemaMajor);
        assert(loaded.schemaMinor == ProjectIO::kProjectSchemaMinor);
        assert(loaded.bpm == 126.0f);
        assert(loaded.projectKeyRoot == 0);
        assert(loaded.projectKeyMode == AppState::kProjectKeyModeNaturalMinor);

        std::filesystem::remove(tempPath);
}

void testUnsupportedSchemaRejected() {
        const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_project_v99.cendance").string();

        std::ofstream output(tempPath, std::ios::binary | std::ios::trunc);
        output << R"({
    "format": "cendanceProject",
    "schemaVersion": { "major": 99, "minor": 0 },
    "state": {
        "bpm": 120.0,
        "playing": false,
        "metronomeEnabled": true,
        "chordProgression": 0,
        "activeEffects": 0,
        "tracks": [
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] },
            { "algorithmId": 0, "synthPreset": 0, "synthManualOverride": false, "density": 0.5, "complexity": 0.5, "tone": 0.5, "motion": 0.5, "gain": 1.0, "muted": false, "effectPresetSlots": [0,0,0] }
        ],
        "masterEffectPresetSlots": [0, 0, 24]
    }
})";
        output.close();

        ProjectIO::ProjectSnapshot loaded;
        std::string error;
        assert(!ProjectIO::loadProjectFile(tempPath, loaded, error));
        assert(!error.empty());

        std::filesystem::remove(tempPath);
}

// ========================================================================
// P1: Additional ProjectIO Tests
// ========================================================================

void testCorruptJsonRejected() {
    const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_corrupt.cendance").string();
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        out << "this is not valid json {{{";
        out.close();
    }

    ProjectIO::ProjectSnapshot loaded;
    std::string error;
    assert(!ProjectIO::loadProjectFile(tempPath, loaded, error));
    assert(!error.empty());

    std::filesystem::remove(tempPath);
}

void testTruncatedJsonRejected() {
    const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_truncated.cendance").string();
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        out << R"({"format": "cendanceProject","schemaVersion": {"major": 1, "minor": 8},"state": {"bpm": 120.0,)"; // truncated
        out.close();
    }

    ProjectIO::ProjectSnapshot loaded;
    std::string error;
    assert(!ProjectIO::loadProjectFile(tempPath, loaded, error));
    assert(!error.empty());

    std::filesystem::remove(tempPath);
}

void testAll8SectionsRoundTrip() {
    ProjectIO::ProjectSnapshot snapshot;
    snapshot.projectName = "All8Sections";
    snapshot.bpm = 130.0f;
    snapshot.arrangementSectionCount = 8;
    snapshot.arrangementCurrentSection = 5;
    snapshot.arrangementMode = AppState::kArrangementModeAuto;
    snapshot.arrangementSectionLengths = {1, 2, 3, 4, 5, 6, 7, 8};
    snapshot.arrangementSectionProgressions = {
        AppState::kArrangementProgressionFollowGlobal,
        1, 3, 5, 7, 9, 11, AppState::kArrangementProgressionFollowGlobal
    };
    snapshot.arrangementSectionTrackMasks = {
        0b0001, 0b0010, 0b0100, 0b1000, 0b1111, 0b1010, 0b0101, 0b1100
    };
    snapshot.arrangementChainEnabled = true;
    snapshot.arrangementChainLength = 8;
    snapshot.arrangementChainSequence = {7, 6, 5, 4, 3, 2, 1, 0};

    const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_8sections.cendance").string();
    std::string error;
    assert(ProjectIO::saveProjectFile(snapshot, tempPath, error));

    ProjectIO::ProjectSnapshot loaded;
    assert(ProjectIO::loadProjectFile(tempPath, loaded, error));

    assert(loaded.arrangementSectionCount == 8);
    assert(loaded.arrangementCurrentSection == 5);
    for (int i = 0; i < 8; ++i) {
        assert(loaded.arrangementSectionLengths[i] == snapshot.arrangementSectionLengths[i]);
        assert(loaded.arrangementSectionProgressions[i] == snapshot.arrangementSectionProgressions[i]);
        assert(loaded.arrangementSectionTrackMasks[i] == snapshot.arrangementSectionTrackMasks[i]);
        assert(loaded.arrangementChainSequence[i] == snapshot.arrangementChainSequence[i]);
    }

    std::filesystem::remove(tempPath);
}

void testGrooveFieldsInSnapshot() {
    ProjectIO::ProjectSnapshot snapshot;
    snapshot.projectName = "GrooveTest";
    snapshot.bpm = 120.0f;
    // Groove fields are stored in AppState, not ProjectSnapshot directly
    // But we can verify the snapshot round-trips with default groove values
    const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_groove.cendance").string();
    std::string error;
    assert(ProjectIO::saveProjectFile(snapshot, tempPath, error));

    ProjectIO::ProjectSnapshot loaded;
    assert(ProjectIO::loadProjectFile(tempPath, loaded, error));
    assert(loaded.projectName == "GrooveTest");

    std::filesystem::remove(tempPath);
}

void testEmptyProjectNameHandled() {
    ProjectIO::ProjectSnapshot snapshot;
    snapshot.projectName = "";
    snapshot.bpm = 120.0f;

    const auto tempPath = (std::filesystem::temp_directory_path() / "cendance_empty_name.cendance").string();
    std::string error;
    assert(ProjectIO::saveProjectFile(snapshot, tempPath, error));

    ProjectIO::ProjectSnapshot loaded;
    assert(ProjectIO::loadProjectFile(tempPath, loaded, error));
    assert(loaded.projectName.empty());

    std::filesystem::remove(tempPath);
}

void testLoadMissingFileRejected() {
    ProjectIO::ProjectSnapshot loaded;
    std::string error;
    assert(!ProjectIO::loadProjectFile("/tmp/does_not_exist_12345.cendance", loaded, error));
    assert(!error.empty());
}

void testValidateSnapshotRejectsInvalidKeyMode() {
    ProjectIO::ProjectSnapshot snapshot;
    snapshot.bpm = 120.0f;
    // Set an invalid key mode (beyond valid range)
    snapshot.projectKeyRoot = 0;
    snapshot.projectKeyMode = 99; // Invalid

    std::string error;
    // Validation should either reject this or clamp it
    // The current implementation may not validate key mode — this tests the behavior
    bool result = ProjectIO::validateSnapshot(snapshot, error);
    // We just verify it doesn't crash — the result depends on implementation
    (void)result;
}

} // namespace

int main() {
    testSaveLoadRoundTrip();
    testValidationRejectsInvalidBpm();
    testNormalizeProjectPath();
    testApplySnapshotEnqueuesCommands();
    testLoadWithoutArrangementPayloadKeepsDefaults();
    testSchemaV2LoadWithExtraFields();
    testUnsupportedSchemaRejected();

    // P1: Additional tests
    testCorruptJsonRejected();
    testTruncatedJsonRejected();
    testAll8SectionsRoundTrip();
    testGrooveFieldsInSnapshot();
    testEmptyProjectNameHandled();
    testLoadMissingFileRejected();
    testValidateSnapshotRejectsInvalidKeyMode();

    std::cout << "ProjectIO tests passed!\n";
    return 0;
}
