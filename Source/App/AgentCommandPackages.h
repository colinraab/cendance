#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executePackages(ExecutionContext& context, const std::vector<std::string>& tokens);
Response executePresets(ExecutionContext& context, const std::vector<std::string>& tokens);

} // namespace AgentCommand
