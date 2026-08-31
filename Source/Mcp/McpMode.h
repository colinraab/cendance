#pragma once

#include "../App/AppState.h"
#include "../App/CommandQueue.h"
#include "../App/ContributionPackage.h"
#include "../App/MeterQueue.h"
#include "P2PToolHandler.h"

#include <functional>
#include <string>

struct McpModeContext {
    AppState& appState;
    CommandQueue& commandQueue;
    MeterQueue& meterQueue;
    ContributionPackage::Library& contributionLibrary;
    P2PToolHandler& p2pToolHandler;
    std::function<std::string(const std::string& action, const std::string& argsJson)> recordFn;
};

int runMcpMode(McpModeContext context);
