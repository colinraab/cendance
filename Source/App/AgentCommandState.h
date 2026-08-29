#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executeHelp();
Response executeState(ExecutionContext& context, bool full);
Response executeCatalog(ExecutionContext& context, const std::string& domain);
Response executeMeters(ExecutionContext& context);
Response executeListen(ExecutionContext& context, double seconds);

} // namespace AgentCommand
