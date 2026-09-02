/*
  ==============================================================================
    cendance - Terminal Generative Music App
  ==============================================================================
*/

#include "App/AppState.h"
#include "App/CliOptions.h"
#include "App/DrumSampleLibrary.h"
#include "App/JuceRuntime.h"
#include "App/MelodicSampleLibrary.h"
#include "App/MeterQueue.h"
#include "App/ProjectIO.h"
#include "App/ProjectIOLoad.h"
#include "App/StartupRuntime.h"
#include "App/StdoutRedirect.h"
#include "Mcp/McpMode.h"
#include "Mcp/P2PToolHandler.h"
#include "Network/P2PClient.h"
#include "Network/PeerDiscovery.h"
#include "Security/PresetSerializer.h"
#include "Security/SecurityManager.h"
#include "UI/TuiApp.h"

#include <iostream>
#include <filesystem>
#include <unistd.h>

int main(int argc, char* argv[])
{
    auto cli = parseCommandLine(argc, argv);
    if (cli.exitCode) return *cli.exitCode;

    StdoutRedirect stdoutRedirect(cli.options.mcpMode);

    // 1. Setup shared app objects
    RuntimeObjects runtime;
    StartupStatus startupStatus;
    StartupProject startup = loadStartupProject(cli.options);
    initializeLibraries(runtime, startupStatus);
    runtime.appState.setPlaying(false);

    // 2. Initialize JUCE MessageManager on a background thread
    JuceRuntime juce(runtime.appState, runtime.commandQueue, runtime.meterQueue,
                     runtime.drumSampleLibrary, runtime.melodicSampleLibrary,
                     cli.options.deviceName);
    juce.start();
    stdoutRedirect.restore();

    const auto toJuceString = [](const std::string& value) {
        return juce::String::fromUTF8(value.data(), static_cast<int>(value.size()));
    };
    const auto makeControlResponse = [&toJuceString](bool ok, const std::string& message) {
        auto* response = new juce::DynamicObject();
        response->setProperty("ok", ok);
        response->setProperty("message", toJuceString(message));
        return juce::JSON::toString(juce::var(response), false).toStdString();
    };

    const auto recordingControl = [&juce, &toJuceString, &makeControlResponse]
                                  (const std::string& action,
                                   const std::string& argsJson) -> std::string {
        if (action == "start") {
            const auto parsed = juce::JSON::parse(toJuceString(argsJson));
            if (!parsed.isObject() || !parsed.hasProperty("path"))
                return makeControlResponse(false, "Recording path is required");

            const std::string path = parsed["path"].toString().toStdString();
            const std::string formatName = parsed.hasProperty("format")
                ? parsed["format"].toString().toStdString()
                : "wav:f32";
            if (path.empty())
                return makeControlResponse(false, "Recording path is required");

            cendance::FileRecorder::Format format;
            if (formatName == "wav:f32")
                format = cendance::FileRecorder::Format::WavF32;
            else if (formatName == "wav:s16")
                format = cendance::FileRecorder::Format::WavS16;
            else if (formatName == "flac:24")
                format = cendance::FileRecorder::Format::Flac24;
            else if (formatName == "flac:16")
                format = cendance::FileRecorder::Format::Flac16;
            else
                return makeControlResponse(false, "Unsupported recording format: " + formatName);

            if (juce.startRecording(path, format))
                return makeControlResponse(true, "Recording started: " + path);
            return makeControlResponse(false, "Failed to start recording: " + path);
        }

        if (action == "stop") {
            juce.stopRecording();
            return makeControlResponse(true, "Recording stopped");
        }

        if (action == "status") {
            const auto status = juce.getRecordingStatus();
            auto* response = new juce::DynamicObject();
            response->setProperty("ok", true);
            response->setProperty("recording", status.recording);
            response->setProperty("duration", status.durationSeconds);
            response->setProperty("overrun", status.overrun);
            response->setProperty("path", toJuceString(status.filePath));
            response->setProperty("samplesWritten", static_cast<int64_t>(status.totalSamplesWritten));
            response->setProperty("samplesDropped", static_cast<int64_t>(status.totalSamplesDropped));
            response->setProperty("error", toJuceString(status.lastError));
            return juce::JSON::toString(juce::var(response), false).toStdString();
        }

        return makeControlResponse(false, "Unknown recording action: " + action);
    };

    // Start recording if --record was specified
    if (!cli.options.recordPath.empty()) {
        cendance::FileRecorder::Format format = cendance::FileRecorder::Format::WavF32;
        if (cli.options.recordFormat == "wav:s16")
            format = cendance::FileRecorder::Format::WavS16;
        else if (cli.options.recordFormat == "flac:24")
            format = cendance::FileRecorder::Format::Flac24;
        else if (cli.options.recordFormat == "flac:16")
            format = cendance::FileRecorder::Format::Flac16;

        if (juce.startRecording(cli.options.recordPath, format)) {
            std::ostream& statusOutput = cli.options.mcpMode ? std::cerr : std::cout;
            statusOutput << "Recording to: " << cli.options.recordPath
                         << " (format: " << cli.options.recordFormat << ")" << std::endl;
        } else {
            std::cerr << "Failed to start recording to: " << cli.options.recordPath << std::endl;
        }
    }

    // 3. Apply startup project or randomize
    applyStartupProjectOrRandomize(runtime, startup, cli.options.loadProjectPath);

    // Initialize sharing infrastructure (available locally; acknowledgement
    // gates signing and exchange operations).
    SecurityManager securityManager;
    securityManager.initialize();
    PresetSerializer presetSerializer;
    P2PClient p2pClient;

    if (cli.options.mcpMode) {
        // 3a. MCP mode — run headless stdio server instead of TUI
        P2PToolHandler p2pHandler(runtime.appState, securityManager, presetSerializer, p2pClient);
        McpModeContext mcpContext{
            runtime.appState, runtime.commandQueue, runtime.meterQueue,
            runtime.contributionLibrary, p2pHandler, recordingControl
        };
        int result = runMcpMode(mcpContext);
        juce.stopStreaming();
        juce.stopRecording();
        juce.stop();
        return result;
    }

    // 3b. Run the Terminal UI on the main thread
    const std::string initialProjectPath = !cli.options.saveProjectPath.empty()
        ? cli.options.saveProjectPath
        : cli.options.loadProjectPath;

    // Initialize peer discovery if requested
    PeerDiscovery peerDiscovery;
    if (cli.options.discoverPeers || cli.options.agentPort > 0) {
        std::string discoveryError;
        if (cli.options.advertiseName.empty()) {
            char hostname[256];
            if (gethostname(hostname, sizeof(hostname)) == 0)
                const_cast<std::string&>(cli.options.advertiseName) = std::string(hostname);
            else
                const_cast<std::string&>(cli.options.advertiseName) = "cendance";
        }
        if (cli.options.agentPort > 0) {
            if (peerDiscovery.startAdvertising(cli.options.agentPort, cli.options.advertiseName, discoveryError)) {
                std::cout << "Peer discovery: advertising on LAN as '" << cli.options.advertiseName << "' port " << cli.options.agentPort << std::endl;
            } else {
                std::cerr << "Peer discovery: failed to advertise: " << discoveryError << std::endl;
            }
        }
        if (cli.options.discoverPeers) {
            if (peerDiscovery.startBrowsing(discoveryError)) {
                std::cout << "Peer discovery: browsing for peers on LAN" << std::endl;
            } else {
                std::cerr << "Peer discovery: failed to browse: " << discoveryError << std::endl;
            }
        }
    }

    TuiApp app(runtime.appState,
               runtime.commandQueue,
               runtime.meterQueue,
               &runtime.drumSampleLibrary,
               &runtime.contributionLibrary,
               &p2pClient,
               &presetSerializer,
               &peerDiscovery,
               initialProjectPath,
               startup.status.message,
               startup.status.isError,
               cli.options.agentPort,
               recordingControl,
               nullptr);
    app.run();

    if (cli.options.saveOnExit) {
        std::string finalSavePath = cli.options.saveProjectPath;
        if (finalSavePath.empty()) {
            finalSavePath = app.getActiveProjectPath();
        }

        if (!finalSavePath.empty()) {
            auto snapshot = ProjectIO::snapshotFromState(runtime.appState);
            snapshot.projectName = std::filesystem::path(finalSavePath).stem().string();

            std::string saveError;
            if (!ProjectIO::saveProjectFile(snapshot, finalSavePath, saveError)) {
                std::cerr << "Failed to save project on exit: " << saveError << std::endl;
            }
        } else {
            std::cerr << "--save-on-exit ignored: no save path available." << std::endl;
        }
    }

    // 4. Shutdown
    juce.stopStreaming();
    juce.stopRecording();
    juce.stop();
    return 0;
}
