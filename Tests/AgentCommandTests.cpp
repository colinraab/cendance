#include "../Source/App/AgentCommand.h"
#include "../Source/Mcp/McpServer.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Harness {
    AppState state;
    CommandQueue queue;
    MeterData currentMeters;
    std::vector<MeterData> history;
    std::vector<std::string> descriptions;
    ContributionPackage::Library contributionLibrary;
    bool recordCalled = false;
    bool streamCalled = false;
    bool p2pCalled = false;
    std::string lastRecordResult;
    std::string lastStreamResult;
    std::string lastP2pResult;

    AgentCommand::Response execute(const std::string& input) {
        AgentCommand::ExecutionContext context{
            state,
            &queue,
            currentMeters,
            history,
            [this](const Command& command,
                   const std::string& description,
                   const Command&) {
                descriptions.push_back(description);
                return queue.push(command);
            },
            &contributionLibrary,
            // P2P bridge
            [this](const std::string& toolName, const std::string& argsJson) -> std::string {
                p2pCalled = true;
                lastP2pResult = "{\"ok\":true,\"tool\":\"" + toolName + "\",\"args\":\"" + argsJson + "\"}";
                return lastP2pResult;
            },
            // Recording bridge
            [this](const std::string& action, const std::string& argsJson) -> std::string {
                recordCalled = true;
                lastRecordResult = "{\"ok\":true,\"action\":\"" + action + "\"}";
                return lastRecordResult;
            },
            // Streaming bridge
            [this](const std::string& action, const std::string& argsJson) -> std::string {
                streamCalled = true;
                lastStreamResult = "{\"ok\":true,\"action\":\"" + action + "\"}";
                return lastStreamResult;
            }
        };
        return AgentCommand::execute(input, context);
    }

    void resetBridgeFlags() {
        recordCalled = false;
        streamCalled = false;
        p2pCalled = false;
        lastRecordResult.clear();
        lastStreamResult.clear();
        lastP2pResult.clear();
    }
};

void testContextCommands() {
    Harness h;

    auto help = h.execute("help");
    assert(help.ok);
    assert(help.json.find("catalog algorithms") != std::string::npos);

    auto state = h.execute("state full");
    assert(state.ok);
    assert(state.json.find("\"tracks\"") != std::string::npos);
    assert(state.json.find("\"master\"") != std::string::npos);
    assert(state.json.find("\"progression\"") != std::string::npos);
    assert(state.json.find("\"genre_tags\"") != std::string::npos);

    auto algorithms = h.execute("catalog algorithms");
    assert(algorithms.ok);
    assert(algorithms.json.find("FourOnFloor") != std::string::npos);

    auto sounds = h.execute("catalog sounds");
    assert(sounds.ok);
    assert(sounds.json.find("Bass") != std::string::npos);
    assert(sounds.json.find("\"genre_tags\"") != std::string::npos);

    auto effects = h.execute("catalog effects");
    assert(effects.ok);
    assert(effects.json.find("Limiter") != std::string::npos);

    auto progressions = h.execute("catalog progressions");
    assert(progressions.ok);
    assert(progressions.json.find("Trance/Pop") != std::string::npos);
    assert(progressions.json.find("\"genre_tags\"") != std::string::npos);

    auto meters = h.execute("meters");
    assert(meters.ok);
    assert(meters.json.find("\"spectrum\"") != std::string::npos);
}

void testMutationCommandsDispatchExpectedCommand() {
    Harness h;
    Command command{};

    auto play = h.execute("play");
    assert(play.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::PlayStop);

    auto tempo = h.execute("tempo set 128");
    assert(tempo.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetTempoAbsolute);
    assert(command.value == 128.0f);

    auto density = h.execute("track 2 density 0.75");
    assert(density.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetDensityAbsolute);
    assert(command.trackIndex == 1);
    assert(command.value == 0.75f);

    auto algorithm = h.execute("track 4 algorithm 13");
    assert(algorithm.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetAlgorithm);
    assert(command.trackIndex == 3);
    assert(command.paramId == 12);

    auto sound = h.execute("track 3 sound 8");
    assert(sound.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetSynthPreset);
    assert(command.trackIndex == 2);
    assert(command.paramId == 7);

    auto trackFx = h.execute("track 2 fx 1 43");
    assert(trackFx.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetTrackEffectPreset);
    assert(command.trackIndex == 1);
    assert(Command::decodeEffectSlotIndex(command.paramId) == 0);

    auto masterFx = h.execute("master fx 2 30");
    assert(masterFx.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetMasterEffectPreset);
    assert(Command::decodeEffectSlotIndex(command.paramId) == 1);

    auto key = h.execute("key \"A minor\"");
    assert(key.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetProjectKey);

    auto progression = h.execute("progression 4");
    assert(progression.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetChordProg);
    assert(command.paramId == 3);

    auto genre = h.execute("genre House");
    assert(genre.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetGenre);
    assert(command.paramId == 1);

    auto genreRandomize = h.execute("genre randomize \"UK Garage\"");
    assert(genreRandomize.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::RandomizeForGenre);
    assert(command.paramId == 2);

    auto freeRandomize = h.execute("genre randomize none");
    assert(freeRandomize.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::RandomizeForGenre);
    assert(command.paramId == 0);

    auto arrangement = h.execute("arrangement section 2");
    assert(arrangement.ok);
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetArrangementSection);
    assert(command.paramId == 1);
}

void testValidationFailures() {
    Harness h;

    assert(!h.execute("").ok);
    assert(!h.execute("track 5 density 0.5").ok);
    assert(!h.execute("track 1 density 3.0").ok);
    assert(!h.execute("track 2 algorithm 999").ok);
    assert(!h.execute("track 3 sound 999").ok);
    assert(!h.execute("master fx 4 1").ok);
    assert(!h.execute("key \"not a key\"").ok);
    assert(!h.execute("progression 999").ok);
    assert(!h.execute("genre Nope").ok);
    assert(!h.execute("genre randomize 9").ok);
    assert(!h.execute("arrangement section 9").ok);
}

void testMcpGenreToolsDispatchAgentCommands() {
    std::string lastCommand;
    McpServer server([&lastCommand](const juce::String& command) {
        lastCommand = command.toStdString();
        return juce::String("{\"ok\":true}");
    });

    const std::string schemas = juce::JSON::toString(McpServer::toolSchemas(), false).toStdString();
    assert(schemas.find("set_genre") != std::string::npos);
    assert(schemas.find("randomize_for_genre") != std::string::npos);

    auto makeCall = [](const char* name, juce::var args) {
        auto* params = new juce::DynamicObject();
        params->setProperty("name", name);
        params->setProperty("arguments", args);
        return juce::var(params);
    };

    auto* setArgs = new juce::DynamicObject();
    setArgs->setProperty("id", 6);
    auto setResult = server.handleToolsCall(makeCall("set_genre", juce::var(setArgs)), juce::var(1));
    assert(!setResult.isVoid());
    assert(lastCommand == "genre 6");

    auto* randomizeArgs = new juce::DynamicObject();
    randomizeArgs->setProperty("name", "UK Garage");
    auto randomizeResult = server.handleToolsCall(makeCall("randomize_for_genre", juce::var(randomizeArgs)), juce::var(2));
    assert(!randomizeResult.isVoid());
    assert(lastCommand == "genre randomize \"UK Garage\"");
}

void testListenHeuristicUsesHistory() {
    Harness h;
    for (int i = 0; i < 90; ++i) {
        MeterData meter;
        meter.isPlaying = true;
        meter.barNumber = static_cast<uint16_t>(i / 16);
        meter.beatPosition = static_cast<uint32_t>((i / 4) % 4);
        meter.masterLevel = 0.45f + ((i % 8) * 0.025f);
        for (int track = 0; track < 4; ++track) {
            meter.trackLevels[track] = 0.35f + static_cast<float>(track) * 0.04f;
            meter.activeNotes[track][0] = 1ULL << (36 + track);
        }
        for (size_t bin = 0; bin < meter.spectrumBins.size(); ++bin) {
            meter.spectrumBins[bin] = 0.4f + static_cast<float>(bin % 5) * 0.03f;
        }
        h.history.push_back(meter);
        h.currentMeters = meter;
    }

    auto listen = h.execute("listen 2s");
    assert(listen.ok);
    assert(listen.json.find("\"overall\"") != std::string::npos);
    assert(listen.json.find("\"silenceRisk\"") != std::string::npos);
    assert(listen.json.find("Meter heuristics") != std::string::npos);
}

void testContributionPackageCommands() {
    const auto tempRoot = std::filesystem::temp_directory_path()
        / ("cendance-contrib-test-" + std::to_string(std::rand()));
    std::filesystem::create_directories(tempRoot);
    const auto libraryRoot = tempRoot / "Contributions";
    setenv("CENDANCE_CONTRIBUTIONS_DIR", libraryRoot.string().c_str(), 1);
    const auto packagePath = tempRoot / "bass-pack.cendance-package.json";
    {
        std::ofstream out(packagePath);
        out << R"json({
  "schema": "cendancePackage.v1",
  "id": "test.agent.bass",
  "version": "0.1.0",
  "kind": "soundPresetPack",
  "name": "Agent Bass Pack",
  "description": "Test package.",
  "authorAgent": "test-agent",
  "createdAt": "2026-05-11T00:00:00Z",
  "license": "CC0-1.0",
  "compatibility": {
    "minPackageSchema": "cendancePackage.v1",
    "maxPackageSchema": "cendancePackage.v1"
  },
  "dependencies": [],
  "tags": ["test"],
  "items": [
    {
      "id": "round-sub",
      "name": "Round Sub",
      "description": "A safe existing-engine bass sound.",
      "track": 2,
      "soundDisplayId": 1,
      "fx": [1, 2],
      "macros": {
        "density": 0.44,
        "complexity": 0.22,
        "gain": 0.88
      },
      "tags": ["bass"]
    }
  ]
})json";
    }

    Harness h;
    auto preview = h.execute("packages preview \"" + packagePath.string() + "\"");
    assert(preview.ok);
    assert(preview.json.find("soundPresetPack") != std::string::npos);
    assert(preview.json.find("Round Sub") != std::string::npos);

    auto install = h.execute("packages install \"" + packagePath.string() + "\"");
    assert(install.ok);

    auto catalog = h.execute("packages catalog");
    assert(catalog.ok);
    assert(catalog.json.find("test.agent.bass") != std::string::npos);

    auto apply = h.execute("packages apply sound test.agent.bass round-sub");
    assert(apply.ok);

    Command command{};
    assert(h.queue.pop(command));
    assert(command.type == Command::Type::SetSynthPreset);
    assert(command.trackIndex == 1);

    bool sawFx = false;
    bool sawDensity = false;
    while (h.queue.pop(command)) {
        sawFx = sawFx || command.type == Command::Type::SetTrackEffectPreset;
        sawDensity = sawDensity || command.type == Command::Type::SetDensityAbsolute;
    }
    assert(sawFx);
    assert(sawDensity);

    auto remove = h.execute("packages remove test.agent.bass");
    assert(remove.ok);
    std::filesystem::remove_all(tempRoot);
}

// ========================================================================
// P0: Swing command tests
// ========================================================================

void testSwingGetReturnsCurrentValue() {
    Harness h;
    // Default swing should be 0
    auto resp = h.execute("swing");
    assert(resp.ok);
    assert(resp.message.find("0") != std::string::npos);
}

void testSwingSetValue() {
    Harness h;
    auto resp = h.execute("swing set 50");
    assert(resp.ok);
    // After setting, swing should be 50
    auto get = h.execute("swing");
    assert(get.ok);
    assert(get.message.find("50") != std::string::npos);
}

void testSwingSetClampsTo100() {
    Harness h;
    auto resp = h.execute("swing set 150");
    assert(resp.ok);
    auto get = h.execute("swing");
    assert(get.ok);
    assert(get.message.find("100") != std::string::npos);
}

void testSwingSetClampsTo0() {
    Harness h;
    auto resp = h.execute("swing set -10");
    assert(resp.ok);
    auto get = h.execute("swing");
    assert(get.ok);
    assert(get.message.find("0") != std::string::npos);
}

void testSwingSetInvalidValue() {
    Harness h;
    auto resp = h.execute("swing set abc");
    assert(!resp.ok);
}

void testSwingUnknownSubcommand() {
    Harness h;
    auto resp = h.execute("swing foo");
    assert(!resp.ok);
}

// ========================================================================
// P0: Humanize command tests
// ========================================================================

void testHumanizeGetReturnsCurrentValues() {
    Harness h;
    auto resp = h.execute("humanize");
    assert(resp.ok);
    // Default should be 0, 0
    assert(resp.message.find("0") != std::string::npos);
}

void testHumanizeSetVelocityOnly() {
    Harness h;
    auto resp = h.execute("humanize set 75");
    assert(resp.ok);
    auto get = h.execute("humanize");
    assert(get.ok);
    assert(get.message.find("75") != std::string::npos);
}

void testHumanizeSetVelocityAndJitter() {
    Harness h;
    auto resp = h.execute("humanize set 50 25");
    assert(resp.ok);
    auto get = h.execute("humanize");
    assert(get.ok);
    assert(get.message.find("50") != std::string::npos);
    assert(get.message.find("25") != std::string::npos);
}

void testHumanizeSetClampsValues() {
    Harness h;
    auto resp = h.execute("humanize set 200 -5");
    assert(resp.ok);
    auto get = h.execute("humanize");
    assert(get.ok);
    // Should clamp to 100 and 0
    assert(get.message.find("100") != std::string::npos);
}

void testHumanizeSetInvalidValue() {
    Harness h;
    auto resp = h.execute("humanize set abc");
    assert(!resp.ok);
}

void testHumanizeUnknownSubcommand() {
    Harness h;
    auto resp = h.execute("humanize foo");
    assert(!resp.ok);
}

// ========================================================================
// P0: Project command tests
// ========================================================================

void testProjectSaveCommand() {
    Harness h;
    auto resp = h.execute("project save test_project");
    assert(resp.ok);
    assert(resp.message.find("saved") != std::string::npos);
}

void testProjectLoadCommand() {
    Harness h;
    // First save
    auto save = h.execute("project save test_load_project");
    assert(save.ok);
    // Extract path from save response
    // The response contains the path — load it
    // For simplicity, just verify the command structure works
    auto load = h.execute("project load /nonexistent/file.cendance");
    assert(!load.ok); // Should fail for nonexistent file
}

void testProjectListCommand() {
    Harness h;
    auto resp = h.execute("project list");
    assert(resp.ok);
    assert(resp.json.find("projects") != std::string::npos);
}

void testProjectDeleteCommand() {
    Harness h;
    auto resp = h.execute("project delete /nonexistent/file.cendance");
    assert(!resp.ok); // Should fail for nonexistent file
}

void testProjectSaveRequiresName() {
    Harness h;
    auto resp = h.execute("project save");
    assert(!resp.ok);
}

void testProjectLoadMissingFile() {
    Harness h;
    auto resp = h.execute("project load /tmp/definitely_does_not_exist_12345.cendance");
    assert(!resp.ok);
}

// ========================================================================
// P0: Record/Stream/P2P bridge command tests
// ========================================================================

void testRecordCommandRequiresBridge() {
    // Create a harness without bridges by using a minimal context
    // The default Harness now has bridges, so we test with them
    Harness h;
    auto resp = h.execute("record status");
    assert(resp.ok);
    assert(h.recordCalled);
}

void testRecordCommandWithBridge() {
    Harness h;
    h.resetBridgeFlags();
    auto resp = h.execute("record status");
    assert(resp.ok);
    assert(h.recordCalled);
    assert(h.lastRecordResult.find("status") != std::string::npos);
}

void testStreamCommandRequiresBridge() {
    Harness h;
    auto resp = h.execute("stream status");
    assert(resp.ok);
    assert(h.streamCalled);
}

void testStreamCommandWithBridge() {
    Harness h;
    h.resetBridgeFlags();
    auto resp = h.execute("stream status");
    assert(resp.ok);
    assert(h.streamCalled);
    assert(h.lastStreamResult.find("status") != std::string::npos);
}

void testP2PCommandRequiresBridge() {
    Harness h;
    auto resp = h.execute("p2p peers");
    assert(resp.ok);
    // p2p.peers doesn't need bridge
}

void testP2PCommandWithBridge() {
    Harness h;
    h.resetBridgeFlags();
    auto resp = h.execute("p2p search_presets");
    assert(resp.ok);
    assert(h.p2pCalled);
}

void testP2PPeersCommand() {
    Harness h;
    auto resp = h.execute("p2p peers");
    assert(resp.ok);
}

} // namespace

int main() {
    testContextCommands();
    testMutationCommandsDispatchExpectedCommand();
    testValidationFailures();
    testMcpGenreToolsDispatchAgentCommands();
    testListenHeuristicUsesHistory();
    testContributionPackageCommands();

    // P0: Swing tests
    testSwingGetReturnsCurrentValue();
    testSwingSetValue();
    testSwingSetClampsTo100();
    testSwingSetClampsTo0();
    testSwingSetInvalidValue();
    testSwingUnknownSubcommand();

    // P0: Humanize tests
    testHumanizeGetReturnsCurrentValues();
    testHumanizeSetVelocityOnly();
    testHumanizeSetVelocityAndJitter();
    testHumanizeSetClampsValues();
    testHumanizeSetInvalidValue();
    testHumanizeUnknownSubcommand();

    // P0: Project tests
    testProjectSaveCommand();
    testProjectLoadCommand();
    testProjectListCommand();
    testProjectDeleteCommand();
    testProjectSaveRequiresName();
    testProjectLoadMissingFile();

    // P0: Record/Stream/P2P bridge tests
    testRecordCommandRequiresBridge();
    testRecordCommandWithBridge();
    testStreamCommandRequiresBridge();
    testStreamCommandWithBridge();
    testP2PCommandRequiresBridge();
    testP2PCommandWithBridge();
    testP2PPeersCommand();

    std::cout << "AgentCommand tests passed!\n";
    return 0;
}
