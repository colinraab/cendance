#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executePlay(ExecutionContext& context);
Response executePause(ExecutionContext& context);
Response executeStop(ExecutionContext& context);
Response executeTempo(ExecutionContext& context, const std::vector<std::string>& tokens);

} // namespace AgentCommand
