#pragma once

#include "AppState.h"
#include "PresetRegistry.h"

#include <random>

namespace StartupProjectInitializer {

void applyRandomizedStartupProject(AppState& appState, std::mt19937& rng);
void applyRandomizedStartupProject(AppState& appState,
                                   const PresetRegistry::Registry& presetRegistry,
                                   std::mt19937& rng);
void applyRandomizedStartupProject(AppState& appState);
void applyRandomizedStartupProject(AppState& appState,
                                   const PresetRegistry::Registry& presetRegistry);

} // namespace StartupProjectInitializer
