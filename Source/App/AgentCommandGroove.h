#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executeSwing(ExecutionContext& context, const std::vector<std::string>& tokens);
Response executeHumanize(ExecutionContext& context, const std::vector<std::string>& tokens);

} // namespace AgentCommand
