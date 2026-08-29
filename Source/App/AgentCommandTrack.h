#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executeTrack(ExecutionContext& context, const std::vector<std::string>& tokens);

} // namespace AgentCommand
