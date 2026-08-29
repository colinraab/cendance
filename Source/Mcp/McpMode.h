#pragma once

#include "../App/AppState.h"
#include "../App/CommandQueue.h"
#include "../App/ContributionPackage.h"
#include "../App/MeterQueue.h"
#include "P2PToolHandler.h"

struct McpModeContext {
    AppState& appState;
    CommandQueue& commandQueue;
    MeterQueue& meterQueue;
    ContributionPackage::Library& contributionLibrary;
    P2PToolHandler& p2pToolHandler;
};

int runMcpMode(McpModeContext context);
