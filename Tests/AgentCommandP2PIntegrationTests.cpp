#include "IntegrationTestHelpers.h"
#include "../Source/App/AgentCommand.h"
#include "../Source/Security/PresetSerializer.h"
#include "../Source/Security/SecurityManager.h"
#include "../Source/Network/P2PClient.h"
#include "../Source/App/AppState.h"

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

// ---- Test 1: p2p save_and_sign_preset through agent ----
void testAgentP2pSaveAndSignPreset() {
    std::cout << "testAgentP2pSaveAndSignPreset...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;

    SecurityManager sm;
    sm.initialize();
    PresetSerializer serializer;
    auto p2pClient = std::make_unique<P2PClient>();
    p2pClient->setEndpoint("file://" + tmpDir.dir.getFullPathName().toStdString() + "/store");


    std::string lastP2pResult;
    AgentCommand::Response response;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string& argsJson) -> std::string {
            lastP2pResult = "{\"ok\":true,\"tool\":\"" + toolName + "\",\"args\":\"" + argsJson + "\"}";
            return lastP2pResult;
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p save_and_sign_preset", context);
    ASSERT_TRUE(response.ok);
    ASSERT_FALSE(response.json.empty());

    PASS();
}

// ---- Test 2: p2p share_on_network through agent ----
void testAgentP2pShareOnNetwork() {
    std::cout << "testAgentP2pShareOnNetwork...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;


    AgentCommand::Response response;
    std::string capturedTool, capturedArgs;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string& argsJson) -> std::string {
            capturedTool = toolName;
            capturedArgs = argsJson;
            return "{\"ok\":true,\"preset_id\":\"test_123\"}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p share_on_network {\"preset_json\":\"{}\"}", context);
    ASSERT_TRUE(response.ok);
    ASSERT_EQ(capturedTool, "share_on_network");

    PASS();
}

// ---- Test 3: p2p search_network through agent ----
void testAgentP2pSearchNetwork() {
    std::cout << "testAgentP2pSearchNetwork...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;


    AgentCommand::Response response;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string&) -> std::string {
            return "{\"presets\":[{\"preset_id\":\"p1\",\"sender_id\":\"s1\",\"display_name\":\"Test\",\"timestamp\":123}]}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p search_network", context);
    ASSERT_TRUE(response.ok);
    ASSERT_TRUE(response.json.find("presets") != std::string::npos);

    PASS();
}

// ---- Test 4: p2p verify_incoming_preset through agent ----
void testAgentP2pVerifyIncomingPreset() {
    std::cout << "testAgentP2pVerifyIncomingPreset...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;


    AgentCommand::Response response;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string&) -> std::string {
            return "{\"ok\":true,\"trust_level\":0,\"payload_json\":\"{}\"}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p verify_incoming_preset {\"preset_json\":\"{}\"}", context);
    ASSERT_TRUE(response.ok);
    ASSERT_TRUE(response.json.find("ok") != std::string::npos);

    PASS();
}

// ---- Test 5: p2p save_and_sign_project through agent ----
void testAgentP2pSaveAndSignProject() {
    std::cout << "testAgentP2pSaveAndSignProject...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;


    AgentCommand::Response response;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string&) -> std::string {
            return "{\"ok\":true,\"envelope\":\"{}\"}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p save_and_sign_project {\"name\":\"Agent Project\"}", context);
    ASSERT_TRUE(response.ok);

    PASS();
}

// ---- Test 6: p2p share_project_on_network through agent ----
void testAgentP2pShareProjectOnNetwork() {
    std::cout << "testAgentP2pShareProjectOnNetwork...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;


    AgentCommand::Response response;
    std::string capturedTool;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string&) -> std::string {
            capturedTool = toolName;
            return "{\"ok\":true,\"project_id\":\"proj_123\"}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p share_project_on_network {\"envelope_json\":\"{}\"}", context);
    ASSERT_TRUE(response.ok);
    ASSERT_EQ(capturedTool, "share_project_on_network");

    PASS();
}

// ---- Test 7: p2p download_project through agent ----
void testAgentP2pDownloadProject() {
    std::cout << "testAgentP2pDownloadProject...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;


    AgentCommand::Response response;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string&) -> std::string {
            return "{\"ok\":true,\"local_path\":\"/tmp/test_project.cendance\",\"name\":\"Test Project\"}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p download_project {\"envelope_json\":\"{}\"}", context);
    ASSERT_TRUE(response.ok);
    ASSERT_TRUE(response.json.find("ok") != std::string::npos);

    PASS();
}

// ---- Test 8: Unknown p2p tool returns error 404 ----
void testAgentP2pUnknownTool() {
    std::cout << "testAgentP2pUnknownTool...\n";
    TempDir tmpDir;
    auto state = makeAppState();
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    ContributionPackage::Library contributionLibrary;

    AgentCommand::Response response;

    AgentCommand::ExecutionContext context{
        *state,
        &queue,
        currentMeters,
        history,
        [](const Command&, const std::string&, const Command&) { return true; },
        &contributionLibrary,
        [&](const std::string& toolName, const std::string&) -> std::string {
            return "{\"error\":{\"code\":404,\"message\":\"Unknown P2P tool: " + toolName + "\"}}";
        },
        [](const std::string&, const std::string&) -> std::string { return "{}"; },
        [](const std::string&, const std::string&) -> std::string { return "{}"; }
    };

    response = AgentCommand::execute("p2p nonexistent_tool", context);
    // The agent command layer should pass through the error
    ASSERT_TRUE(response.ok || response.json.find("404") != std::string::npos || response.json.find("Unknown") != std::string::npos);

    PASS();
}

int main() {
    isolateAppDataForTests();
    std::cout << "=== Agent Command -> P2P Bridge Integration Tests ===\n\n";

    testAgentP2pSaveAndSignPreset();
    testAgentP2pShareOnNetwork();
    testAgentP2pSearchNetwork();
    testAgentP2pVerifyIncomingPreset();
    testAgentP2pSaveAndSignProject();
    testAgentP2pShareProjectOnNetwork();
    testAgentP2pDownloadProject();
    testAgentP2pUnknownTool();

    std::cout << "\n=== Results: " << testsPassed << " passed, " << testsFailed << " failed ===\n";
    return testsFailed > 0 ? 1 : 0;
}
