#pragma once

#include "AppState.h"
#include "CommandQueue.h"
#include "ContributionPackage.h"
#include "DrumSampleLibrary.h"
#include "MelodicSampleLibrary.h"
#include "MeterQueue.h"
#include "PresetRegistry.h"
#include "ProjectIO.h"

#include <string>

struct StartupStatus {
    std::string message;
    bool isError = false;
};

struct StartupProject {
    ProjectIO::ProjectSnapshot snapshot;
    bool hasSnapshot = false;
    StartupStatus status;
};

struct RuntimeObjects {
    AppState appState;
    CommandQueue commandQueue;
    MeterQueue meterQueue;
    DrumSampleLibrary drumSampleLibrary;
    MelodicSampleLibrary melodicSampleLibrary;
    ContributionPackage::Library contributionLibrary;
};

struct CliOptions;

StartupProject loadStartupProject(const CliOptions& options);
void initializeLibraries(RuntimeObjects& runtime, StartupStatus& status);
void applyStartupProjectOrRandomize(RuntimeObjects& runtime,
                                    StartupProject& startup,
                                    const std::string& loadProjectPath);
