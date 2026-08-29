#include "IntegrationTestHelpers.h"
#include "../Source/Mcp/P2PToolHandler.h"
#include "../Source/Security/PresetSerializer.h"
#include "../Source/Security/SecurityManager.h"
#include "../Source/Network/P2PClient.h"
#include "../Source/App/AppState.h"
#include "../Source/App/CustomAlgorithmPreset.h"
#include "../Source/Config/ToSGuard.h"

#include <cassert>
#include <iostream>

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

#define ASSERT_GE(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (!(_a >= _b)) { \
        std::cerr << "  FAIL: " << #a << " >= " << #b << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define PASS() do { testsPassed++; std::cout << "  PASS\n"; } while(0)

static std::unique_ptr<AppState> makeAppState() {
    auto state = std::make_unique<AppState>();
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

// Helper to build JSON args using juce::DynamicObject (avoids manual escaping)
static std::string makeArgs(std::initializer_list<std::pair<const char*, std::string>> kv) {
    auto obj = std::make_unique<juce::DynamicObject>();
    for (auto& [key, val] : kv) {
        obj->setProperty(key, juce::String(val));
    }
    return juce::JSON::toString(juce::var(obj.release()), false).toStdString();
}

// ---- Test 1: get_tos_status returns false initially ----
void testGetTosStatusInitial() {
    std::cout << "testGetTosStatusInitial...\n";
    ToSGuard::configFile().deleteFile();
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);

    auto result = handler.handle("get_tos_status", "{}");
    ASSERT_FALSE(result.isEmpty());

    auto parsed = juce::JSON::parse(result);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_FALSE(varGetBool(parsed, "accepted"));

    PASS();
}

// ---- Test 2: After accept(), get_tos_status returns true ----
void testTosStatusAfterAccept() {
    std::cout << "testTosStatusAfterAccept...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);

    // Clean up any existing ToS config first
    ToSGuard::configFile().deleteFile();

    ASSERT_TRUE(ToSGuard::accept());

    auto result = handler.handle("get_tos_status", "{}");
    auto parsed = juce::JSON::parse(result);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "accepted"));

    // Cleanup ToS config
    ToSGuard::configFile().deleteFile();

    PASS();
}

// ---- Test 3: ToS status requires a real boolean acceptance flag ----
void testTosStatusRejectsNonBooleanAcceptance() {
    std::cout << "testTosStatusRejectsNonBooleanAcceptance...\n";
    ToSGuard::configDirectory().createDirectory();
    {
        std::ofstream out(ToSGuard::configFile().getFullPathName().toStdString(),
                          std::ios::binary | std::ios::trunc);
        out << R"({"tos_accepted":"true","tos_accepted_at":"2026-01-01T00:00:00Z"})";
    }

    ASSERT_FALSE(ToSGuard::isAccepted());
    ASSERT_TRUE(ToSGuard::acceptedTimestamp().empty());

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 4: save_and_sign_preset returns valid envelope ----
void testSaveAndSignPreset() {
    std::cout << "testSaveAndSignPreset...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    auto result = handler.handle("save_and_sign_preset", "{}");
    ASSERT_FALSE(result.isEmpty());

    auto parsed = juce::JSON::parse(result);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varHasProperty(parsed, "envelope"));

    std::string envelope = varGetString(parsed, "envelope");
    ASSERT_FALSE(envelope.empty());

    // Verify it's valid JSON with expected fields
    auto envParsed = juce::JSON::parse(juce::String(envelope));
    ASSERT_TRUE(envParsed.isObject());
    ASSERT_TRUE(varIsInt(envParsed, "content_type"));
    ASSERT_TRUE(varIsString(envParsed, "header"));
    ASSERT_TRUE(varIsString(envParsed, "payload"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 4: share_on_network returns ok ----
void testShareOnNetwork() {
    std::cout << "testShareOnNetwork...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    // First create an envelope
    auto signResult = handler.handle("save_and_sign_preset", "{}");
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    // Share it using properly escaped JSON args
    auto args = makeArgs({{"preset_json", envelope}});
    auto shareResult = handler.handle("share_on_network", args);
    ASSERT_FALSE(shareResult.isEmpty());

    auto parsed = juce::JSON::parse(shareResult);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "ok"));
    ASSERT_TRUE(varHasProperty(parsed, "preset_id"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 5: search_network returns published items ----
void testSearchNetwork() {
    std::cout << "testSearchNetwork...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    // Create and share a preset
    auto signResult = handler.handle("save_and_sign_preset", "{}");
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");
    handler.handle("share_on_network", makeArgs({{"preset_json", envelope}}));

    // Search
    auto searchResult = handler.handle("search_network", "{}");
    ASSERT_FALSE(searchResult.isEmpty());

    auto parsed = juce::JSON::parse(searchResult);
    ASSERT_TRUE(parsed.isObject());
    auto* presets = varGet(parsed, "presets").getArray();
    ASSERT_TRUE(presets != nullptr);
    ASSERT_GE(presets->size(), (int)1);

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 6: verify_incoming_preset on fresh envelope ----
void testVerifyIncomingPreset() {
    std::cout << "testVerifyIncomingPreset...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    // Create an envelope
    auto signResult = handler.handle("save_and_sign_preset", "{}");
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    // Verify it
    auto verifyResult = handler.handle("verify_incoming_preset",
        makeArgs({{"preset_json", envelope}}));
    ASSERT_FALSE(verifyResult.isEmpty());

    auto parsed = juce::JSON::parse(verifyResult);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "ok"));
    ASSERT_EQ(varGetInt(parsed, "trust_level"), 0); // Verified = 0

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 7: save_and_sign_sample with test WAV ----
void testSaveAndSignSample() {
    std::cout << "testSaveAndSignSample...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    std::string wavPath = tmpDir.dir.getChildFile("mcptool_test.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    auto result = handler.handle("save_and_sign_sample",
        makeArgs({{"path", wavPath}, {"name", "MCP Test Sample"}}));
    ASSERT_FALSE(result.isEmpty());

    auto parsed = juce::JSON::parse(result);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varHasProperty(parsed, "envelope"));
    ASSERT_TRUE(varHasProperty(parsed, "sha256"));
    ASSERT_TRUE(varHasProperty(parsed, "format"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 8: share_sample_on_network ----
void testShareSampleOnNetwork() {
    std::cout << "testShareSampleOnNetwork...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    std::string wavPath = tmpDir.dir.getChildFile("share_test.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    auto signResult = handler.handle("save_and_sign_sample",
        makeArgs({{"path", wavPath}}));
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    auto shareResult = handler.handle("share_sample_on_network",
        makeArgs({{"sample_json", envelope}}));
    ASSERT_FALSE(shareResult.isEmpty());

    auto parsed = juce::JSON::parse(shareResult);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "ok"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 9: search_samples returns published sample ----
void testSearchSamples() {
    std::cout << "testSearchSamples...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    std::string wavPath = tmpDir.dir.getChildFile("search_test.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    auto signResult = handler.handle("save_and_sign_sample",
        makeArgs({{"path", wavPath}, {"name", "Searchable Sample"}}));
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");
    handler.handle("share_sample_on_network",
        makeArgs({{"sample_json", envelope}}));

    auto searchResult = handler.handle("search_samples", "{}");
    ASSERT_FALSE(searchResult.isEmpty());

    auto parsed = juce::JSON::parse(searchResult);
    ASSERT_TRUE(parsed.isObject());
    auto* samples = varGet(parsed, "samples").getArray();
    ASSERT_TRUE(samples != nullptr);
    ASSERT_GE(samples->size(), (int)1);

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 10: download_sample full cycle ----
void testDownloadSample() {
    std::cout << "testDownloadSample...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    std::string wavPath = tmpDir.dir.getChildFile("download_test.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    auto signResult = handler.handle("save_and_sign_sample",
        makeArgs({{"path", wavPath}, {"name", "Download Test"}}));
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    auto shareResult = handler.handle("share_sample_on_network",
        makeArgs({{"sample_json", envelope}}));
    auto shareParsed = juce::JSON::parse(shareResult);
    std::string sampleId = varGetString(shareParsed, "sample_id");

    ASSERT_FALSE(sampleId.empty());

    auto downloadResult = handler.handle("download_sample",
        makeArgs({{"sample_id", sampleId}}));
    ASSERT_FALSE(downloadResult.isEmpty());

    auto parsed = juce::JSON::parse(downloadResult);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "ok"));
    ASSERT_TRUE(varHasProperty(parsed, "local_path"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 11: list_downloaded_samples ----
void testListDownloadedSamples() {
    std::cout << "testListDownloadedSamples...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    std::string wavPath = tmpDir.dir.getChildFile("list_test.wav").getFullPathName().toStdString();
    ASSERT_TRUE(generateTestWav(wavPath));

    auto signResult = handler.handle("save_and_sign_sample",
        makeArgs({{"path", wavPath}, {"name", "List Test"}}));
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    auto shareResult = handler.handle("share_sample_on_network",
        makeArgs({{"sample_json", envelope}}));
    auto shareParsed = juce::JSON::parse(shareResult);
    std::string sampleId = varGetString(shareParsed, "sample_id");

    handler.handle("download_sample", makeArgs({{"sample_id", sampleId}}));

    auto listResult = handler.handle("list_downloaded_samples", "{}");
    ASSERT_FALSE(listResult.isEmpty());

    auto parsed = juce::JSON::parse(listResult);
    ASSERT_TRUE(parsed.isObject());
    auto* downloads = varGet(parsed, "downloads").getArray();
    ASSERT_TRUE(downloads != nullptr);
    ASSERT_GE(downloads->size(), (int)1);

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 12: save_and_sign_project ----
void testSaveAndSignProject() {
    std::cout << "testSaveAndSignProject...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    auto result = handler.handle("save_and_sign_project",
        makeArgs({{"name", "MCP Project Test"}}));
    ASSERT_FALSE(result.isEmpty());

    auto parsed = juce::JSON::parse(result);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varHasProperty(parsed, "envelope"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 13: share_project_on_network ----
void testShareProjectOnNetwork() {
    std::cout << "testShareProjectOnNetwork...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    auto signResult = handler.handle("save_and_sign_project",
        makeArgs({{"name", "Share Project Test"}}));
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    auto shareResult = handler.handle("share_project_on_network",
        makeArgs({{"envelope_json", envelope}}));
    ASSERT_FALSE(shareResult.isEmpty());

    auto parsed = juce::JSON::parse(shareResult);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "ok"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 14: download_project ----
void testDownloadProject() {
    std::cout << "testDownloadProject...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    ToSGuard::accept();

    auto signResult = handler.handle("save_and_sign_project",
        makeArgs({{"name", "Download Project Test"}}));
    auto signParsed = juce::JSON::parse(signResult);
    std::string envelope = varGetString(signParsed, "envelope");

    auto shareResult = handler.handle("share_project_on_network",
        makeArgs({{"envelope_json", envelope}}));
    auto shareParsed = juce::JSON::parse(shareResult);
    std::string projectId = varGetString(shareParsed, "project_id");

    ASSERT_FALSE(projectId.empty());

    auto downloadResult = handler.handle("download_project",
        makeArgs({{"envelope_json", envelope}}));
    ASSERT_FALSE(downloadResult.isEmpty());

    auto parsed = juce::JSON::parse(downloadResult);
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(varGetBool(parsed, "ok"));
    ASSERT_TRUE(varHasProperty(parsed, "local_path"));

    ToSGuard::configFile().deleteFile();
    PASS();
}

// ---- Test 15: ToS guard blocks sharing tools ----
void testTosGuardBlocksSharing() {
    std::cout << "testTosGuardBlocksSharing...\n";
    TempDir tmpDir;
    SecurityManager sm;
    sm.initialize();
    auto state = makeAppState();
    PresetSerializer serializer;
    auto client = std::make_unique<P2PClient>();
    client->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");

    P2PToolHandler handler(*state, sm, serializer, *client);
    // Do NOT accept ToS — clean up any existing config
    ToSGuard::configFile().deleteFile();

    // Each sharing tool should return error 4001
    const char* tools[] = {
        "save_and_sign_preset",
        "share_on_network",
        "search_network",
        "verify_incoming_preset",
        "save_and_sign_sample",
        "share_sample_on_network",
    };

    for (const char* tool : tools) {
        auto result = handler.handle(tool, "{}");
        ASSERT_FALSE(result.isEmpty());
        auto parsed = juce::JSON::parse(result);
        ASSERT_TRUE(parsed.isObject());
        int code = varGetInt(parsed, "code");
        ASSERT_EQ(code, 4001);
    }

    PASS();
}

int main() {
    isolateToSConfigForTests();
    std::cout << "=== MCP Tool Handler Integration Tests ===\n\n";

    testGetTosStatusInitial();
    testTosStatusAfterAccept();
    testTosStatusRejectsNonBooleanAcceptance();
    testSaveAndSignPreset();
    testShareOnNetwork();
    testSearchNetwork();
    testVerifyIncomingPreset();
    testSaveAndSignSample();
    testShareSampleOnNetwork();
    testSearchSamples();
    testDownloadSample();
    testListDownloadedSamples();
    testSaveAndSignProject();
    testShareProjectOnNetwork();
    testDownloadProject();
    testTosGuardBlocksSharing();

    std::cout << "\n=== Results: " << testsPassed << " passed, " << testsFailed << " failed ===\n";
    return testsFailed > 0 ? 1 : 0;
}
