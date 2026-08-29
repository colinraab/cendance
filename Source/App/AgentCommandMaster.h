#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executeMaster(ExecutionContext& context, const std::vector<std::string>& tokens);

} // namespace AgentCommand
