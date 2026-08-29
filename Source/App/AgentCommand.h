#pragma once

#include "AppState.h"
#include "CommandQueue.h"
#include "ContributionPackage.h"
#include "MeterQueue.h"

#include <functional>
#include <string>
#include <vector>

namespace AgentCommand {

struct Response {
    bool ok = false;
    std::string message;
    std::string json;
};

using DispatchCommandFn = std::function<bool(const Command& command,
                                             const std::string& description,
                                             const Command& undoCommand)>;

struct ExecutionContext {
    AppState& appState;
    CommandQueue* commandQueue = nullptr;
    const MeterData& currentMeters;
    const std::vector<MeterData>& meterHistory;
    DispatchCommandFn dispatchCommand;
    ContributionPackage::Library* contributionLibrary = nullptr;
    // P2P bridge: if set, called for p2p.* commands instead of local dispatch
    std::function<std::string(const std::string& toolName, const std::string& argsJson)> p2pFn;
    // Recording bridge: if set, called for record.* commands
    std::function<std::string(const std::string& action, const std::string& argsJson)> recordFn;
    // Streaming bridge: if set, called for stream.* commands
    std::function<std::string(const std::string& action, const std::string& argsJson)> streamFn;
};

Response execute(const std::string& input, ExecutionContext context);

} // namespace AgentCommand
