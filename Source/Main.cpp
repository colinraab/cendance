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
#include "Config/ToSGuard.h"
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
            std::cout << "Recording to: " << cli.options.recordPath
                      << " (format: " << cli.options.recordFormat << ")" << std::endl;
        } else {
            std::cerr << "Failed to start recording to: " << cli.options.recordPath << std::endl;
        }
    }

    // Start streaming if --audio-stream was specified
    if (!cli.options.audioStreamTarget.empty()) {
        cendance::StreamSink::Format streamFmt = cendance::StreamSink::Format::F32LE;
        if (cli.options.audioStreamFormat == "s16le")
            streamFmt = cendance::StreamSink::Format::S16LE;

        std::string streamTarget = cli.options.audioStreamTarget;
        if (juce.startStreaming(
                [streamTarget](const void* data, size_t bytes) -> bool {
                    if (streamTarget == "stdout") {
                        return fwrite(data, 1, bytes, stdout) == bytes;
                    }
                    // TODO: TCP socket support
                    return false;
                }, streamFmt)) {
            std::cout << "Streaming to: " << cli.options.audioStreamTarget
                      << " (format: " << cli.options.audioStreamFormat << ")" << std::endl;
        } else {
            std::cerr << "Failed to start streaming to: " << cli.options.audioStreamTarget << std::endl;
        }
    }

    // 3. Apply startup project or randomize
    applyStartupProjectOrRandomize(runtime, startup, cli.options.loadProjectPath);

    // Initialize P2P infrastructure (always available; ToS gates actual use)
    SecurityManager securityManager;
    securityManager.initialize();
    PresetSerializer presetSerializer;
    P2PClient p2pClient;

    if (cli.options.mcpMode) {
        // 3a. MCP mode — run headless stdio server instead of TUI
        P2PToolHandler p2pHandler(runtime.appState, securityManager, presetSerializer, p2pClient);
        McpModeContext mcpContext{
            runtime.appState, runtime.commandQueue, runtime.meterQueue,
            runtime.contributionLibrary, p2pHandler
        };
        int result = runMcpMode(mcpContext);
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
               // Recording callback
               [&juce](const std::string& action, const std::string& argsJson) -> std::string {
                   if (action == "start") {
                       // Parse args: expect "path" and optionally "format"
                       juce::var parsed;
                       try { parsed = juce::JSON::parse(juce::String(argsJson)); } catch (...) {}
                       std::string path = "output.wav";
                       std::string fmt = "wav:f32";
                       if (parsed.isObject()) {
                           if (parsed.hasProperty("path")) path = parsed["path"].toString().toStdString();
                           if (parsed.hasProperty("format")) fmt = parsed["format"].toString().toStdString();
                       }
                       cendance::FileRecorder::Format format = cendance::FileRecorder::Format::WavF32;
                       if (fmt == "wav:s16") format = cendance::FileRecorder::Format::WavS16;
                       else if (fmt == "flac:24") format = cendance::FileRecorder::Format::Flac24;
                       else if (fmt == "flac:16") format = cendance::FileRecorder::Format::Flac16;
                       if (juce.startRecording(path, format))
                           return "{\"ok\":true,\"message\":\"Recording started: " + path + "\"}";
                       return "{\"ok\":false,\"message\":\"Failed to start recording\"}";
                   } else if (action == "stop") {
                       juce.stopRecording();
                       return "{\"ok\":true,\"message\":\"Recording stopped\"}";
                   } else if (action == "status") {
                       auto st = juce.getRecordingStatus();
                       auto* res = new juce::DynamicObject();
                       res->setProperty("ok", true);
                       res->setProperty("recording", st.recording);
                       res->setProperty("duration", st.durationSeconds);
                       res->setProperty("overrun", st.overrun);
                       res->setProperty("path", juce::String(st.filePath));
                       res->setProperty("samplesWritten", static_cast<int64_t>(st.totalSamplesWritten));
                       res->setProperty("samplesDropped", static_cast<int64_t>(st.totalSamplesDropped));
                       return juce::JSON::toString(juce::var(res), false).toStdString();
                   }
                   return "{\"ok\":false,\"message\":\"Unknown recording action: " + action + "\"}";
               },
               // Streaming callback
               [&juce](const std::string& action, const std::string& argsJson) -> std::string {
                   if (action == "start") {
                       juce::var parsed;
                       try { parsed = juce::JSON::parse(juce::String(argsJson)); } catch (...) {}
                       std::string target = "stdout";
                       std::string fmt = "f32le";
                       if (parsed.isObject()) {
                           if (parsed.hasProperty("target")) target = parsed["target"].toString().toStdString();
                           if (parsed.hasProperty("format")) fmt = parsed["format"].toString().toStdString();
                       }
                       cendance::StreamSink::Format format = cendance::StreamSink::Format::F32LE;
                       if (fmt == "s16le") format = cendance::StreamSink::Format::S16LE;
                       if (juce.startStreaming(
                               [target](const void* data, size_t bytes) -> bool {
                                   if (target == "stdout") return fwrite(data, 1, bytes, stdout) == bytes;
                                   return false;
                               }, format))
                           return "{\"ok\":true,\"message\":\"Streaming started: " + target + "\"}";
                       return "{\"ok\":false,\"message\":\"Failed to start streaming\"}";
                   } else if (action == "stop") {
                       juce.stopStreaming();
                       return "{\"ok\":true,\"message\":\"Streaming stopped\"}";
                   } else if (action == "status") {
                       auto st = juce.getStreamingStatus();
                       auto* res = new juce::DynamicObject();
                       res->setProperty("ok", true);
                       res->setProperty("streaming", st.streaming);
                       res->setProperty("duration", st.durationSeconds);
                       res->setProperty("overrun", st.overrun);
                       res->setProperty("samplesWritten", static_cast<int64_t>(st.totalSamplesWritten));
                       res->setProperty("samplesDropped", static_cast<int64_t>(st.totalSamplesDropped));
                       return juce::JSON::toString(juce::var(res), false).toStdString();
                   }
                   return "{\"ok\":false,\"message\":\"Unknown streaming action: " + action + "\"}";
               });
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
