#pragma once

#include "AgentCommand.h"
#include "AgentCommandUtils.h"

namespace AgentCommand {

Response executeKey(ExecutionContext& context, const std::vector<std::string>& tokens);
Response executeProgression(ExecutionContext& context, const std::vector<std::string>& tokens);
Response executeGenre(ExecutionContext& context, const std::vector<std::string>& tokens);
Response executeArrangement(ExecutionContext& context, const std::vector<std::string>& tokens);
Response executeProject(ExecutionContext& context, const std::vector<std::string>& tokens);

} // namespace AgentCommand
