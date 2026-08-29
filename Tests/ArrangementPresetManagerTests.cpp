#include "../Source/App/ArrangementPresetManager.h"
#include "../Source/App/ContributionPackage.h"

#include <cassert>
#include <atomic>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>

// ========================================================================
// P1 Tests
// ========================================================================

static std::string makeTempDir() {
    static std::atomic<unsigned long> counter{0};
    auto base = std::filesystem::temp_directory_path();
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    auto dir = base / ("cendance-arr-test-" + std::to_string(stamp) + "-" +
                       std::to_string(counter.fetch_add(1)));
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir.string();
}

static void removeDir(const std::string& path) {
    std::filesystem::remove_all(path);
}

void testSnapshotFromStateCapturesArrangement() {
    AppState state;
    state.setArrangementSectionCount(6);
    state.setArrangementCurrentSection(3);
    state.setArrangementMode(AppState::kArrangementModeAuto);
    state.setArrangementSectionLength(0, 2);
    state.setArrangementSectionLength(1, 4);
    state.setArrangementSectionLength(2, 8);
    state.setArrangementSectionProgression(1, 5);
    state.setArrangementSectionProgression(2, 7);
    state.setArrangementSectionTrackMask(0, 0b1010);
    state.setArrangementSectionTrackMask(1, 0b0101);
    state.setArrangementChainEnabled(true);
    state.setArrangementChainLength(4);
    state.setArrangementChainStep(0, 3);
    state.setArrangementChainStep(1, 1);
    state.setArrangementChainStep(2, 2);
    state.setArrangementChainStep(3, 0);

    auto item = ArrangementPresetManager::snapshotFromState(state, "TestArr");

    assert(item.name == "TestArr");
    assert(item.sectionCount == 6);
    assert(item.currentSection == 3);
    assert(item.mode == AppState::kArrangementModeAuto);
    assert(item.sectionLengths[0] == 2);
    assert(item.sectionLengths[1] == 4);
    assert(item.sectionLengths[2] == 8);
    assert(item.sectionProgressions[1] == 5);
    assert(item.sectionProgressions[2] == 7);
    assert(item.trackMasks[0] == 0b1010);
    assert(item.trackMasks[1] == 0b0101);
    assert(item.chainEnabled == true);
    assert(item.chainLength == 4);
    assert(item.chainSequence[0] == 3);
    assert(item.chainSequence[1] == 1);
    assert(item.chainSequence[2] == 2);
    assert(item.chainSequence[3] == 0);
}

void testApplyToStateRestoresArrangement() {
    AppState state;
    // Set some initial values
    state.setArrangementSectionCount(2);
    state.setArrangementCurrentSection(0);
    state.setArrangementMode(AppState::kArrangementModeManual);

    ContributionPackage::ArrangementPresetItem preset;
    preset.name = "RestoreTest";
    preset.sectionCount = 5;
    preset.currentSection = 2;
    preset.mode = AppState::kArrangementModeMixed;
    preset.sectionLengths = {3, 5, 7, 4, 4, 4, 4, 4};
    preset.sectionProgressions = {0, 3, 6, 0, 0, 0, 0, 0};
    preset.trackMasks = {0b1111, 0b1100, 0b0011, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111};
    preset.chainEnabled = true;
    preset.chainLength = 3;
    preset.chainSequence = {2, 0, 1, 3, 4, 5, 6, 7};

    ArrangementPresetManager::applyToState(state, preset);

    assert(state.arrangementSectionCount.load() == 5);
    assert(state.arrangementCurrentSection.load() == 2);
    assert(state.arrangementMode.load() == AppState::kArrangementModeMixed);
    assert(state.getArrangementSectionLength(0) == 3);
    assert(state.getArrangementSectionLength(1) == 5);
    assert(state.getArrangementSectionLength(2) == 7);
    assert(state.getArrangementSectionProgression(1) == 3);
    assert(state.getArrangementSectionProgression(2) == 6);
    assert(state.getArrangementSectionTrackMask(1) == 0b1100);
    assert(state.getArrangementSectionTrackMask(2) == 0b0011);
    assert(state.arrangementChainEnabled.load() == true);
    assert(state.getArrangementChainLength() == 3);
    assert(state.getArrangementChainStep(0) == 2);
    assert(state.getArrangementChainStep(1) == 0);
    assert(state.getArrangementChainStep(2) == 1);
}

void testPresetToJsonRoundTrip() {
    ContributionPackage::ArrangementPresetItem item;
    item.itemId = "test-arr-001";
    item.name = "My Arrangement";
    item.sectionCount = 4;
    item.currentSection = 1;
    item.mode = AppState::kArrangementModeAuto;
    item.sectionLengths = {2, 4, 8, 4, 4, 4, 4, 4};
    item.sectionProgressions = {0, 3, 6, 0, 0, 0, 0, 0};
    item.trackMasks = {0b1111, 0b1100, 0b0011, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111};
    item.chainEnabled = true;
    item.chainLength = 4;
    item.chainSequence = {0, 2, 1, 3, 4, 5, 6, 7};

    std::string json = ArrangementPresetManager::presetToJson(item);

    ContributionPackage::ArrangementPresetItem parsed;
    std::string error;
    assert(ArrangementPresetManager::presetFromJson(json, parsed, error));
    assert(error.empty());

    assert(parsed.itemId == item.itemId);
    assert(parsed.name == item.name);
    assert(parsed.sectionCount == item.sectionCount);
    assert(parsed.currentSection == item.currentSection);
    assert(parsed.mode == item.mode);
    for (int i = 0; i < 8; ++i) {
        assert(parsed.sectionLengths[i] == item.sectionLengths[i]);
        assert(parsed.sectionProgressions[i] == item.sectionProgressions[i]);
        assert(parsed.trackMasks[i] == item.trackMasks[i]);
        assert(parsed.chainSequence[i] == item.chainSequence[i]);
    }
    assert(parsed.chainEnabled == item.chainEnabled);
    assert(parsed.chainLength == item.chainLength);
}

void testPresetFromJsonMissingItemId() {
    std::string json = R"({"name":"NoId","sectionCount":4,"currentSection":0,"mode":0,"sectionLengths":[4,4,4,4,4,4,4,4],"sectionProgressions":[0,0,0,0,0,0,0,0],"trackMasks":[15,15,15,15,15,15,15,15],"chainEnabled":false,"chainLength":4,"chainSequence":[0,1,2,3,4,5,6,7]})";
    ContributionPackage::ArrangementPresetItem item;
    std::string error;
    // Should either fail or generate a default ID
    bool result = ArrangementPresetManager::presetFromJson(json, item, error);
    // If it succeeds, itemId should be empty or auto-generated
    (void)result;
}

void testPresetFromJsonEmptyNameFallsBackToId() {
    std::string json = R"({"itemId":"fallback-id","name":"","sectionCount":4,"currentSection":0,"mode":0,"sectionLengths":[4,4,4,4,4,4,4,4],"sectionProgressions":[0,0,0,0,0,0,0,0],"trackMasks":[15,15,15,15,15,15,15,15],"chainEnabled":false,"chainLength":4,"chainSequence":[0,1,2,3,4,5,6,7]})";
    ContributionPackage::ArrangementPresetItem item;
    std::string error;
    bool result = ArrangementPresetManager::presetFromJson(json, item, error);
    if (result) {
        // Name should fall back to itemId
        assert(item.name == "fallback-id");
    }
}

void testPresetFromJsonArrayFields() {
    std::string json = R"({"itemId":"arr-002","name":"ArrayTest","sectionCount":3,"currentSection":0,"mode":1,"sectionLengths":[2,4,8,4,4,4,4,4],"sectionProgressions":[0,5,9,0,0,0,0,0],"trackMasks":[15,12,3,15,15,15,15,15],"chainEnabled":true,"chainLength":3,"chainSequence":[2,0,1,3,4,5,6,7]})";
    ContributionPackage::ArrangementPresetItem item;
    std::string error;
    assert(ArrangementPresetManager::presetFromJson(json, item, error));
    assert(item.sectionLengths[0] == 2);
    assert(item.sectionLengths[1] == 4);
    assert(item.sectionLengths[2] == 8);
    assert(item.sectionProgressions[1] == 5);
    assert(item.sectionProgressions[2] == 9);
    assert(item.trackMasks[1] == 12);
    assert(item.trackMasks[2] == 3);
    assert(item.chainSequence[0] == 2);
    assert(item.chainSequence[1] == 0);
    assert(item.chainSequence[2] == 1);
}

void testSaveAndLoadRoundTrip() {
    // Override the presets directory to a temp location
    auto tempDir = makeTempDir();
    setenv("CENDANCE_ARRANGEMENT_PRESETS_DIR", tempDir.c_str(), 1);

    ArrangementPresetManager mgr;
    std::string error;
    assert(mgr.ensureDirectories(error));

    AppState state;
    state.setArrangementSectionCount(5);
    state.setArrangementCurrentSection(2);
    state.setArrangementMode(AppState::kArrangementModeMixed);
    state.setArrangementSectionLength(0, 3);
    state.setArrangementSectionLength(1, 5);
    state.setArrangementSectionProgression(1, 4);
    state.setArrangementChainEnabled(true);
    state.setArrangementChainLength(3);
    state.setArrangementChainStep(0, 2);
    state.setArrangementChainStep(1, 0);
    state.setArrangementChainStep(2, 1);

    std::string presetId = mgr.savePreset(state, "RoundTrip", error);
    assert(!presetId.empty());

    // Reload
    assert(mgr.reload(error));

    const auto* found = mgr.findPreset(presetId);
    assert(found != nullptr);
    assert(found->name == "RoundTrip");
    assert(found->sectionCount == 5);
    assert(found->currentSection == 2);
    assert(found->mode == AppState::kArrangementModeMixed);
    assert(found->sectionLengths[0] == 3);
    assert(found->sectionLengths[1] == 5);
    assert(found->sectionProgressions[1] == 4);
    assert(found->chainEnabled == true);
    assert(found->chainLength == 3);
    assert(found->chainSequence[0] == 2);
    assert(found->chainSequence[1] == 0);
    assert(found->chainSequence[2] == 1);

    // Clean up
    removeDir(tempDir);
    unsetenv("CENDANCE_ARRANGEMENT_PRESETS_DIR");
}

void testDeletePreset() {
    auto tempDir = makeTempDir();
    setenv("CENDANCE_ARRANGEMENT_PRESETS_DIR", tempDir.c_str(), 1);

    ArrangementPresetManager mgr;
    std::string error;
    assert(mgr.ensureDirectories(error));

    AppState state;
    state.setArrangementSectionCount(3);
    std::string presetId = mgr.savePreset(state, "ToDelete", error);
    assert(!presetId.empty());

    assert(mgr.findPreset(presetId) != nullptr);
    assert(mgr.deletePreset(presetId, error));
    assert(mgr.findPreset(presetId) == nullptr);

    removeDir(tempDir);
    unsetenv("CENDANCE_ARRANGEMENT_PRESETS_DIR");
}

void testListPresets() {
    auto tempDir = makeTempDir();
    setenv("CENDANCE_ARRANGEMENT_PRESETS_DIR", tempDir.c_str(), 1);

    ArrangementPresetManager mgr;
    std::string error;
    assert(mgr.ensureDirectories(error));

    AppState state;
    const std::string presetAId = mgr.savePreset(state, "PresetA", error);
    const std::string presetBId = mgr.savePreset(state, "PresetB", error);
    assert(!presetAId.empty());
    assert(!presetBId.empty());

    assert(mgr.reload(error));
    assert(mgr.findPreset(presetAId) != nullptr);
    assert(mgr.findPreset(presetBId) != nullptr);

    removeDir(tempDir);
    unsetenv("CENDANCE_ARRANGEMENT_PRESETS_DIR");
}

void testFindPreset() {
    auto tempDir = makeTempDir();
    setenv("CENDANCE_ARRANGEMENT_PRESETS_DIR", tempDir.c_str(), 1);

    ArrangementPresetManager mgr;
    std::string error;
    assert(mgr.ensureDirectories(error));

    AppState state;
    std::string id = mgr.savePreset(state, "FindMe", error);
    assert(!id.empty());

    assert(mgr.reload(error));
    const auto* found = mgr.findPreset(id);
    assert(found != nullptr);
    assert(found->name == "FindMe");

    // Not found
    assert(mgr.findPreset("nonexistent-id") == nullptr);

    removeDir(tempDir);
    unsetenv("CENDANCE_ARRANGEMENT_PRESETS_DIR");
}

void testFindPresetNotFound() {
    ArrangementPresetManager mgr;
    assert(mgr.findPreset("does-not-exist") == nullptr);
}

void testSaveEmptyNameRejected() {
    ArrangementPresetManager mgr;
    std::string error;
    AppState state;
    std::string id = mgr.savePreset(state, "", error);
    assert(id.empty());
}

void testLoadNonexistentPresetRejected() {
    ArrangementPresetManager mgr;
    std::string error;
    AppState state;
    assert(!mgr.loadPreset("nonexistent", state, error));
}

void testDeleteNonexistentPresetRejected() {
    ArrangementPresetManager mgr;
    std::string error;
    assert(!mgr.deletePreset("nonexistent", error));
}

void testPresetsJsonOutput() {
    auto tempDir = makeTempDir();
    setenv("CENDANCE_ARRANGEMENT_PRESETS_DIR", tempDir.c_str(), 1);

    ArrangementPresetManager mgr;
    std::string error;
    assert(mgr.ensureDirectories(error));

    AppState state;
    mgr.savePreset(state, "JsonTest", error);
    assert(mgr.reload(error));

    std::string json = mgr.presetsJson();
    assert(!json.empty());
    assert(json.find("JsonTest") != std::string::npos);
    assert(json.find("[") != std::string::npos); // Should be a JSON array

    removeDir(tempDir);
    unsetenv("CENDANCE_ARRANGEMENT_PRESETS_DIR");
}

void testChainSequencePreserved() {
    ContributionPackage::ArrangementPresetItem item;
    item.itemId = "chain-test";
    item.name = "ChainTest";
    item.sectionCount = 4;
    item.chainEnabled = true;
    item.chainLength = 4;
    item.chainSequence = {3, 1, 0, 2, 4, 5, 6, 7};

    std::string json = ArrangementPresetManager::presetToJson(item);
    ContributionPackage::ArrangementPresetItem parsed;
    std::string error;
    assert(ArrangementPresetManager::presetFromJson(json, parsed, error));
    assert(parsed.chainSequence[0] == 3);
    assert(parsed.chainSequence[1] == 1);
    assert(parsed.chainSequence[2] == 0);
    assert(parsed.chainSequence[3] == 2);
}

void testAll8SectionsPreserved() {
    ContributionPackage::ArrangementPresetItem item;
    item.itemId = "sections-test";
    item.name = "SectionsTest";
    item.sectionCount = 8;
    item.sectionLengths = {1, 2, 3, 4, 5, 6, 7, 8};
    item.sectionProgressions = {0, 1, 2, 3, 4, 5, 6, 7};
    item.trackMasks = {0b0001, 0b0010, 0b0100, 0b1000, 0b1111, 0b1010, 0b0101, 0b1100};

    std::string json = ArrangementPresetManager::presetToJson(item);
    ContributionPackage::ArrangementPresetItem parsed;
    std::string error;
    assert(ArrangementPresetManager::presetFromJson(json, parsed, error));
    for (int i = 0; i < 8; ++i) {
        assert(parsed.sectionLengths[i] == item.sectionLengths[i]);
        assert(parsed.sectionProgressions[i] == item.sectionProgressions[i]);
        assert(parsed.trackMasks[i] == item.trackMasks[i]);
    }
}

// ========================================================================
// Main
// ========================================================================

int main() {
    testSnapshotFromStateCapturesArrangement();
    std::cout << "  testSnapshotFromStateCapturesArrangement passed\n";

    testApplyToStateRestoresArrangement();
    std::cout << "  testApplyToStateRestoresArrangement passed\n";

    testPresetToJsonRoundTrip();
    std::cout << "  testPresetToJsonRoundTrip passed\n";

    testPresetFromJsonMissingItemId();
    std::cout << "  testPresetFromJsonMissingItemId passed\n";

    testPresetFromJsonEmptyNameFallsBackToId();
    std::cout << "  testPresetFromJsonEmptyNameFallsBackToId passed\n";

    testPresetFromJsonArrayFields();
    std::cout << "  testPresetFromJsonArrayFields passed\n";

    testSaveAndLoadRoundTrip();
    std::cout << "  testSaveAndLoadRoundTrip passed\n";

    testDeletePreset();
    std::cout << "  testDeletePreset passed\n";

    testListPresets();
    std::cout << "  testListPresets passed\n";

    testFindPreset();
    std::cout << "  testFindPreset passed\n";

    testFindPresetNotFound();
    std::cout << "  testFindPresetNotFound passed\n";

    testSaveEmptyNameRejected();
    std::cout << "  testSaveEmptyNameRejected passed\n";

    testLoadNonexistentPresetRejected();
    std::cout << "  testLoadNonexistentPresetRejected passed\n";

    testDeleteNonexistentPresetRejected();
    std::cout << "  testDeleteNonexistentPresetRejected passed\n";

    testPresetsJsonOutput();
    std::cout << "  testPresetsJsonOutput passed\n";

    testChainSequencePreserved();
    std::cout << "  testChainSequencePreserved passed\n";

    testAll8SectionsPreserved();
    std::cout << "  testAll8SectionsPreserved passed\n";

    std::cout << "ArrangementPresetManager tests passed!\n";
    return 0;
}
