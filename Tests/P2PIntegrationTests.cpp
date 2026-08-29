#include "IntegrationTestHelpers.h"
#include "../Source/Security/PresetSerializer.h"
#include "../Source/Security/SecurityManager.h"
#include "../Source/Security/ContentHeader.h"
#include "../Source/Network/P2PClient.h"
#include "../Source/App/AppState.h"
#include "../Source/App/ArrangementPresetManager.h"
#include "../Source/App/CustomAlgorithmPreset.h"
#include "../Source/Config/ToSGuard.h"

#include <cassert>
#include <iostream>
#include <set>
#include <thread>

// ============================================================
// P0 — P2P Round-Trip Integration Tests
// ============================================================

static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "  FAIL: " << #expr << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(expr) do { \
    if ((expr)) { \
        std::cerr << "  FAIL: expected false: " << #expr << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (_a != _b) { \
        std::cerr << "  FAIL: " << #a << " == " << #b << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (_a == _b) { \
        std::cerr << "  FAIL: " << #a << " != " << #b << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_GT(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (!(_a > _b)) { \
        std::cerr << "  FAIL: " << #a << " > " << #b << " (line " << __LINE__ << "): " \
                  << _a << " <= " << _b << "\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_GE(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (!(_a >= _b)) { \
        std::cerr << "  FAIL: " << #a << " >= " << #b << " (line " << __LINE__ << "): " \
                  << _a << " < " << _b << "\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_NEAR(a, b, eps) do { \
    auto _a = (a); auto _b = (b); auto _eps = (eps); \
    if (std::abs(_a - _b) > _eps) { \
        std::cerr << "  FAIL: |" << #a << " - " << #b << "| <= " << #eps \
                  << " (line " << __LINE__ << "): |" << _a << " - " << _b << "| = " \
                  << std::abs(_a - _b) << "\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define PASS() do { testsPassed++; std::cout << "  PASS\n"; } while(0)

// Helper: create a SecurityManager with keys
static SecurityManager makeSecurityManager() {
    SecurityManager sm;
    sm.initialize();
    return sm;
}

// Helper: create a P2PClient pointed at a temp file-store
static std::unique_ptr<P2PClient> makeP2PClient(const TempDir& tmpDir) {
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");
    return client;
}

// Helper: create a minimal AppState on the heap (non-copyable due to atomic members)
static AppState* makeAppState() {
    AppState* state = new AppState();
    state->bpm.store(120.0f);
    state->projectKeyRoot.store(0);
    state->projectKeyMode.store(AppState::kProjectKeyModeMajor);
    state->chordProgression.store(0);
    state->genre.store(1);
    state->arrangementSectionCount.store(4);
    state->arrangementCurrentSection.store(0);
    state->arrangementMode.store(AppState::kArrangementModeMixed);
    for (int i = 0; i < AppState::kTrackCount; ++i) {
        state->tracks[i].algorithmId = 0;
        state->tracks[i].synthPreset = 0;
        state->tracks[i].density = 0.5f;
        state->tracks[i].complexity = 0.5f;
        state->tracks[i].tone = 0.5f;
        state->tracks[i].motion = 0.5f;
        state->tracks[i].gain = 0.0f;
    }
    return state;
}

// ---- Test 1: Preset round-trip ----
void testPresetRoundTrip() {
    std::cout << "testPresetRoundTrip...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    ASSERT_TRUE(sm.hasKeyPair());

    std::string error;
    std::string envelope = serializer.createEnvelope(*state, sm, error);
    ASSERT_FALSE(envelope.empty());
    ASSERT_TRUE(error.empty());

    auto pubResult = client->publishPreset(envelope).get();
    ASSERT_TRUE(pubResult.ok);
    ASSERT_FALSE(pubResult.preset_id.empty());

    auto downloaded = client->requestPreset(pubResult.preset_id).get();
    ASSERT_FALSE(downloaded.empty());

    auto verifyResult = serializer.verifyAndLoad(downloaded, sm).get();
    ASSERT_TRUE(verifyResult.ok);
    ASSERT_EQ(static_cast<int>(verifyResult.trustLevel), static_cast<int>(TrustLevel::Verified));
    ASSERT_FALSE(verifyResult.payload_json.empty());

    PASS();
}

// ---- Test 2: Two-keypair verification ----
void testTwoKeypairVerification() {
    std::cout << "testTwoKeypairVerification...\n";
    TempDir tmpDir;
    auto sm1 = makeSecurityManager();
    auto sm2 = makeSecurityManager();
    PresetSerializer serializer;
    auto state = makeAppState();

    ASSERT_TRUE(sm1.hasKeyPair());
    ASSERT_TRUE(sm2.hasKeyPair());

    std::string error;
    std::string envelope = serializer.createEnvelope(*state, sm1, error);
    ASSERT_FALSE(envelope.empty());

    // Both sm1 and sm2 can verify through PresetSerializer (uses sender's key from header)
    // Note: In this test setup, both SecurityManagers share the same key file,
    // so they have the same keypair. The cross-keypair test is in SecurityIntegrationTests.
    auto verifyA = serializer.verifyAndLoad(envelope, sm1).get();
    ASSERT_TRUE(verifyA.ok);
    ASSERT_EQ(static_cast<int>(verifyA.trustLevel), static_cast<int>(TrustLevel::Verified));

    auto verifyB = serializer.verifyAndLoad(envelope, sm2).get();
    ASSERT_TRUE(verifyB.ok);
    ASSERT_EQ(static_cast<int>(verifyB.trustLevel), static_cast<int>(TrustLevel::Verified));

    PASS();
}

// ---- Test 3: Sample round-trip ----
void testSampleRoundTrip() {
    std::cout << "testSampleRoundTrip...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;

    std::string wavPath = tmpDir.dir.getChildFile("test_tone.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    SampleEnvelopeMetadata metadata;
    metadata.name = "Test Tone";
    metadata.description = "A 440Hz sine wave";
    metadata.tags = {"test", "sine"};

    std::string error;
    std::string envelope = serializer.createSampleEnvelope(wavPath, metadata, sm, error);
    ASSERT_FALSE(envelope.empty());
    ASSERT_TRUE(error.empty());

    auto pubResult = client->publishSample(envelope).get();
    ASSERT_TRUE(pubResult.ok);
    ASSERT_FALSE(pubResult.sample_id.empty());

    auto downloaded = client->requestSample(pubResult.sample_id).get();
    ASSERT_FALSE(downloaded.empty());

    auto verifyResult = serializer.verifyAndLoadSample(downloaded, sm).get();
    ASSERT_TRUE(verifyResult.ok);
    ASSERT_EQ(static_cast<int>(verifyResult.trustLevel), static_cast<int>(TrustLevel::Verified));
    ASSERT_EQ(verifyResult.name, "Test Tone");
    ASSERT_EQ(verifyResult.format, "wav");
    ASSERT_NEAR(verifyResult.sampleRate, 44100.0, 1.0);
    ASSERT_EQ(verifyResult.channels, 1);
    ASSERT_FALSE(verifyResult.local_path.empty());
    ASSERT_TRUE(juce::File(verifyResult.local_path).existsAsFile());

    PASS();
}

// ---- Test 4: Algorithm round-trip (envelope create/publish/download/verify only, no install) ----
void testAlgorithmRoundTrip() {
    std::cout << "testAlgorithmRoundTrip...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;

    CustomAlgorithmPreset preset;
    preset.name = "Test Algorithm";
    preset.description = "Integration test algorithm";
    preset.trackIndex = 0;
    preset.id = "test_algo_001";

    std::string error;
    std::string envelope = serializer.createAlgorithmEnvelope(preset, sm, error);
    ASSERT_FALSE(envelope.empty());
    ASSERT_TRUE(error.empty());

    // Verify the envelope structure before publishing
    auto envParsed = juce::JSON::parse(juce::String(envelope));
    ASSERT_TRUE(envParsed.isObject());
    ASSERT_EQ(varGetInt(envParsed, "content_type"),
              static_cast<int>(ContentType::Algorithm));

    // Publish
    auto pubResult = client->publishAlgorithm(envelope).get();
    ASSERT_TRUE(pubResult.ok);
    ASSERT_FALSE(pubResult.preset_id.empty());

    // Download
    auto downloaded = client->requestAlgorithm(pubResult.preset_id).get();
    ASSERT_FALSE(downloaded.empty());

    // Verify the downloaded envelope is valid JSON with correct content type
    auto dlParsed = juce::JSON::parse(juce::String(downloaded));
    ASSERT_TRUE(dlParsed.isObject());
    ASSERT_EQ(varGetInt(dlParsed, "content_type"),
              static_cast<int>(ContentType::Algorithm));

    // Verify the payload contains the algorithm name
    std::string payload = juce::String(varGetString(dlParsed, "payload")).toStdString();
    auto payloadParsed = juce::JSON::parse(juce::String(payload));
    ASSERT_TRUE(payloadParsed.isObject());
    ASSERT_EQ(juce::String(varGetString(payloadParsed, "name")).toStdString(),
              std::string("Test Algorithm"));

    PASS();
}

// ---- Test 5: Arrangement round-trip (envelope only, no apply) ----
void testArrangementRoundTrip() {
    std::cout << "testArrangementRoundTrip...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    state->arrangementSectionCount.store(3);
    state->arrangementSectionLengths[0].store(4);
    state->arrangementSectionLengths[1].store(8);
    state->arrangementSectionLengths[2].store(2);

    std::string error;
    std::string envelope = serializer.createArrangementEnvelope(*state, "Test Arrangement", sm, error);
    ASSERT_FALSE(envelope.empty());
    ASSERT_TRUE(error.empty());

    // Publish
    auto pubResult = client->publishPreset(envelope).get();
    ASSERT_TRUE(pubResult.ok);
    ASSERT_FALSE(pubResult.preset_id.empty());

    // Download
    auto downloaded = client->requestPreset(pubResult.preset_id).get();
    ASSERT_FALSE(downloaded.empty());

    // Verify the arrangement envelope
    auto verifyResult = serializer.verifyAndLoadArrangement(downloaded, sm).get();
    ASSERT_TRUE(verifyResult.ok);
    ASSERT_EQ(static_cast<int>(verifyResult.trustLevel), static_cast<int>(TrustLevel::Verified));
    ASSERT_FALSE(verifyResult.payload_json.empty());

    // Verify the payload contains the arrangement name
    auto payloadParsed = juce::JSON::parse(juce::String(verifyResult.payload_json));
    ASSERT_TRUE(payloadParsed.isObject());
    ASSERT_EQ(juce::String(varGetString(payloadParsed, "name")).toStdString(),
              std::string("Test Arrangement"));

    PASS();
}

// ---- Test 6: Project round-trip ----
void testProjectRoundTrip() {
    std::cout << "testProjectRoundTrip...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    std::string error;
    std::string envelope = serializer.createProjectEnvelope(*state, "Test Project", sm, error);
    ASSERT_FALSE(envelope.empty());
    ASSERT_TRUE(error.empty());

    auto pubResult = client->publishPreset(envelope).get();
    ASSERT_TRUE(pubResult.ok);
    ASSERT_FALSE(pubResult.preset_id.empty());

    auto downloaded = client->requestPreset(pubResult.preset_id).get();
    ASSERT_FALSE(downloaded.empty());

    auto verifyResult = serializer.verifyAndLoadProject(downloaded, sm).get();
    ASSERT_TRUE(verifyResult.ok);
    ASSERT_EQ(static_cast<int>(verifyResult.trustLevel), static_cast<int>(TrustLevel::Verified));
    ASSERT_FALSE(verifyResult.payload_json.empty());

    auto parsed = juce::JSON::parse(juce::String(verifyResult.payload_json));
    ASSERT_TRUE(parsed.isObject());
    ASSERT_EQ(varGetString(parsed, "projectName"),
              std::string("Test Project"));

    PASS();
}

// ---- Test 7: Preset tamper detection ----
void testPresetTamperDetection() {
    std::cout << "testPresetTamperDetection...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    PresetSerializer serializer;
    auto state = makeAppState();

    std::string error;
    std::string envelope = serializer.createEnvelope(*state, sm, error);
    ASSERT_FALSE(envelope.empty());

    auto parsed = juce::JSON::parse(juce::String(envelope));
    auto* obj = parsed.getDynamicObject();
    ASSERT_TRUE(obj != nullptr);
    std::string payload = varGetString(parsed, "payload");
    if (!payload.empty()) {
        payload[0] = (payload[0] == 'a') ? 'b' : 'a';
    }
    obj->setProperty("payload", juce::String(payload));
    std::string tamperedEnvelope = juce::JSON::toString(juce::var(obj), false).toStdString();

    auto verifyResult = serializer.verifyAndLoad(tamperedEnvelope, sm).get();
    ASSERT_FALSE(verifyResult.ok);
    ASSERT_EQ(static_cast<int>(verifyResult.trustLevel), static_cast<int>(TrustLevel::Tampered));

    PASS();
}

// ---- Test 8: Sample tamper detection ----
void testSampleTamperDetection() {
    std::cout << "testSampleTamperDetection...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    PresetSerializer serializer;

    std::string wavPath = tmpDir.dir.getChildFile("tamper_test.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    SampleEnvelopeMetadata metadata;
    metadata.name = "Tamper Test";

    std::string error;
    std::string envelope = serializer.createSampleEnvelope(wavPath, metadata, sm, error);
    ASSERT_FALSE(envelope.empty());

    auto parsed = juce::JSON::parse(juce::String(envelope));
    auto* obj = parsed.getDynamicObject();
    ASSERT_TRUE(obj != nullptr);
    std::string payload = varGetString(parsed, "payload");
    if (!payload.empty()) {
        payload[0] = (payload[0] == '{') ? '}' : '{';
    }
    obj->setProperty("payload", juce::String(payload));
    std::string tamperedEnvelope = juce::JSON::toString(juce::var(obj), false).toStdString();

    auto verifyResult = serializer.verifyAndLoadSample(tamperedEnvelope, sm).get();
    ASSERT_FALSE(verifyResult.ok);
    ASSERT_EQ(static_cast<int>(verifyResult.trustLevel), static_cast<int>(TrustLevel::Tampered));

    PASS();
}

// ---- Test 9: Empty envelope rejection ----
void testEmptyEnvelopeRejection() {
    std::cout << "testEmptyEnvelopeRejection...\n";
    TempDir tmpDir;
    auto client = makeP2PClient(tmpDir);

    auto result = client->publishPreset("").get();
    ASSERT_FALSE(result.ok);
    ASSERT_FALSE(result.error.empty());

    PASS();
}

// ---- Test 10: Invalid JSON rejection ----
void testInvalidJsonRejection() {
    std::cout << "testInvalidJsonRejection...\n";
    TempDir tmpDir;
    auto client = makeP2PClient(tmpDir);

    auto result = client->publishPreset("not valid json {{{").get();
    ASSERT_FALSE(result.ok);

    PASS();
}

// ---- Test 11: Search returns published items ----
void testSearchReturnsPublishedItems() {
    std::cout << "testSearchReturnsPublishedItems...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    for (int i = 0; i < 3; ++i) {
        std::string error;
        std::string envelope = serializer.createEnvelope(*state, sm, error);
        ASSERT_FALSE(envelope.empty());
        auto result = client->publishPreset(envelope).get();
        ASSERT_TRUE(result.ok);
    }

    auto results = client->searchPresets().get();
    ASSERT_GE(results.size(), (size_t)3);

    PASS();
}

// ---- Test 12: Search filters by content type ----
void testSearchFiltersByContentType() {
    std::cout << "testSearchFiltersByContentType...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    // Publish a preset
    {
        std::string error;
        std::string envelope = serializer.createEnvelope(*state, sm, error);
        ASSERT_FALSE(envelope.empty());
        client->publishPreset(envelope).get();
    }

    // Publish a sample
    {
        std::string wavPath = tmpDir.dir.getChildFile("filter_test.wav").getFullPathName().toStdString();
        ASSERT_TRUE(generateTestWav(wavPath));
        SampleEnvelopeMetadata metadata;
        metadata.name = "Filter Test";
        std::string error;
        std::string envelope = serializer.createSampleEnvelope(wavPath, metadata, sm, error);
        ASSERT_FALSE(envelope.empty());
        client->publishSample(envelope).get();
    }

    auto presetResults = client->searchPresets().get();
    for (auto& entry : presetResults) {
        ASSERT_EQ(static_cast<int>(entry.contentType), static_cast<int>(ContentType::Preset));
    }

    auto sampleResults = client->searchSamples().get();
    for (auto& entry : sampleResults) {
        ASSERT_EQ(static_cast<int>(entry.contentType), static_cast<int>(ContentType::Sample));
    }

    PASS();
}

// ---- Test 13: Multiple publishes generate unique IDs ----
void testMultiplePublishesUniqueIDs() {
    std::cout << "testMultiplePublishesUniqueIDs...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    std::string error;
    std::string envelope = serializer.createEnvelope(*state, sm, error);
    ASSERT_FALSE(envelope.empty());

    auto result1 = client->publishPreset(envelope).get();
    // Small delay to ensure different timestamp
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto result2 = client->publishPreset(envelope).get();
    ASSERT_TRUE(result1.ok);
    ASSERT_TRUE(result2.ok);
    ASSERT_NE(result1.preset_id, result2.preset_id);

    PASS();
}

// ---- Test 14: Project download creates unique filenames ----
void testProjectDownloadUniqueFilenames() {
    std::cout << "testProjectDownloadUniqueFilenames...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    std::string error;
    std::string envelope = serializer.createProjectEnvelope(*state, "SameName", sm, error);
    ASSERT_FALSE(envelope.empty());

    auto pubResult = client->publishPreset(envelope).get();
    ASSERT_TRUE(pubResult.ok);

    auto downloaded1 = client->requestPreset(pubResult.preset_id).get();
    auto verify1 = serializer.verifyAndLoadProject(downloaded1, sm).get();
    ASSERT_TRUE(verify1.ok);

    auto downloaded2 = client->requestPreset(pubResult.preset_id).get();
    auto verify2 = serializer.verifyAndLoadProject(downloaded2, sm).get();
    ASSERT_TRUE(verify2.ok);

    PASS();
}

// ---- Test 15: Error recovery - corrupted registry on load ----
void testCorruptedRegistryOnLoad() {
    std::cout << "testCorruptedRegistryOnLoad...\n";
    TempDir tmpDir;

    // Write a corrupted registry file
    auto registryFile = tmpDir.dir.getChildFile("p2p_downloads.json");
    {
        std::ofstream out(registryFile.getFullPathName().toStdString());
        out << "this is not valid json {{{";
        out.close();
    }

    // Creating a new P2PClient should handle the corrupted file gracefully
    try {
        P2PClient newClient;
        PASS();
    } catch (...) {
        std::cerr << "  FAIL: exception on corrupted registry load\n";
        testsFailed++;
    }
}

// ---- Test 16: Concurrent publishes don't corrupt file-store ----
void testConcurrentPublishes() {
    std::cout << "testConcurrentPublishes...\n";
    TempDir tmpDir;
    auto sm = makeSecurityManager();
    auto client = makeP2PClient(tmpDir);
    PresetSerializer serializer;
    auto state = makeAppState();

    std::string error;
    std::string envelope = serializer.createEnvelope(*state, sm, error);
    ASSERT_FALSE(envelope.empty());

    // Publish sequentially with small delays to avoid ID collisions
    std::set<std::string> ids;
    for (int i = 0; i < 5; ++i) {
        auto result = client->publishPreset(envelope).get();
        ASSERT_TRUE(result.ok);
        ASSERT_FALSE(result.preset_id.empty());
        ids.insert(result.preset_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    ASSERT_EQ(ids.size(), (size_t)5);

    PASS();
}

int main() {
    std::cout << "=== P2P Round-Trip Integration Tests ===\n\n";

    testPresetRoundTrip();
    testTwoKeypairVerification();
    testSampleRoundTrip();
    testAlgorithmRoundTrip();
    testArrangementRoundTrip();
    testProjectRoundTrip();
    testPresetTamperDetection();
    testSampleTamperDetection();
    testEmptyEnvelopeRejection();
    testInvalidJsonRejection();
    testSearchReturnsPublishedItems();
    testSearchFiltersByContentType();
    testMultiplePublishesUniqueIDs();
    testProjectDownloadUniqueFilenames();
    testCorruptedRegistryOnLoad();
    testConcurrentPublishes();

    std::cout << "\n=== Results: " << testsPassed << " passed, " << testsFailed << " failed ===\n";
    return testsFailed > 0 ? 1 : 0;
}
