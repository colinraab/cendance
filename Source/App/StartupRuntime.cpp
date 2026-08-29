#include "StartupRuntime.h"

#include "CliOptions.h"
#include "ProjectIOLoad.h"
#include "StartupProjectInitializer.h"

#include <string>

using namespace juce;

namespace {

void appendError(StartupStatus& status, const std::string& text) {
    if (status.message.empty()) {
        status.message = text;
    } else {
        status.message += " " + text;
    }
    status.isError = true;
}

} // namespace

StartupProject loadStartupProject(const CliOptions& options) {
    StartupProject startup;

    std::string pathError;
    if (!options.loadProjectPath.empty()) {
        std::string normalized;
        if (!ProjectIO::normalizeProjectPath(options.loadProjectPath, normalized, pathError, true)) {
            startup.status.message = "Startup load skipped: invalid path (" + pathError + ")";
            startup.status.isError = true;
        } else {
            if (!ProjectIO::loadProjectFile(normalized, startup.snapshot, pathError)) {
                startup.status.message = "Startup load failed: " + pathError;
                startup.status.isError = true;
            } else {
                startup.hasSnapshot = true;
            }
        }
    }

    if (!options.saveProjectPath.empty()) {
        std::string normalized;
        if (!ProjectIO::normalizeProjectPath(options.saveProjectPath, normalized, pathError, true)) {
            startup.status.message = "Invalid --save-project path: " + pathError;
            startup.status.isError = true;
        }
    }

    return startup;
}

void initializeLibraries(RuntimeObjects& runtime, StartupStatus& status) {
    {
        std::string preloadError;
        if (!runtime.drumSampleLibrary.preloadEmbeddedDrumKits(preloadError)) {
            appendError(status, "Embedded drum kits failed to load: " + preloadError);
        }
    }

    {
        std::string preloadError;
        if (!runtime.melodicSampleLibrary.preloadEmbeddedSamples(preloadError)) {
            appendError(status, "Embedded melodic samples failed to load: " + preloadError);
        }
    }

    {
        std::string scanError;
        if (!runtime.drumSampleLibrary.rescanGlobalDirectory(scanError)) {
            appendError(status, "Drum sample scan failed: " + scanError);
        }
    }

    {
        std::string contributionError;
        if (!runtime.contributionLibrary.reloadInstalled(contributionError)) {
            appendError(status, "Contribution package scan failed: " + contributionError);
        }
        PresetRegistry::globalRegistry().rebuild(&runtime.contributionLibrary);
        PresetRegistry::publishCustomEffectsToAudio(PresetRegistry::globalRegistry());
    }
}

void applyStartupProjectOrRandomize(RuntimeObjects& runtime,
                                    StartupProject& startup,
                                    const std::string& loadProjectPath) {
    bool startupSnapshotApplied = false;
    if (startup.hasSnapshot) {
        std::string applyError;
        if (!ProjectIO::applySnapshotToCommandQueue(startup.snapshot,
                                                    runtime.appState,
                                                    runtime.commandQueue,
                                                    applyError,
                                                    true)) {
            startup.status.message = "Startup load apply failed: " + applyError;
            startup.status.isError = true;
        } else {
            startupSnapshotApplied = true;
            startup.status.message = "Loaded startup project: " + loadProjectPath;
            startup.status.isError = false;
        }
    }

    if (!startupSnapshotApplied) {
        StartupProjectInitializer::applyRandomizedStartupProject(runtime.appState, PresetRegistry::globalRegistry());
        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
            const auto preset = runtime.appState.tracks[track].synthPreset.load(std::memory_order_relaxed);
            if (!runtime.commandQueue.push(Command{Command::Type::SetSynthPreset, track, preset, 0.0f})) {
                if (startup.status.message.empty()) {
                    startup.status.message = "Random sound apply skipped: command queue full.";
                } else {
                    startup.status.message += " Random sound apply skipped: command queue full.";
                }
                startup.status.isError = true;
                break;
            }
        }
        if (startup.status.message.empty()) {
            startup.status.message = "Started with a randomized project. Restart the app to get a new idea!";
        } else {
            startup.status.message += " Fallback: started with a randomized project. Restart the app to get a new idea!";
        }
    }
}
