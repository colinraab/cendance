#include "AgentCommand.h"

#include "AgentCommandState.h"
#include "AgentCommandTransport.h"
#include "AgentCommandTrack.h"
#include "AgentCommandMaster.h"
#include "AgentCommandMusical.h"
#include "AgentCommandPackages.h"
#include "AgentCommandGroove.h"
#include "AgentCommandUtils.h"

#include <juce_core/juce_core.h>

namespace AgentCommand {

Response execute(const std::string& input, ExecutionContext context) {
    const std::string trimmed = trimCopy(input);
    if (trimmed.empty()) {
        return makeResponse(false, "Empty command.");
    }

    std::string tokenizeError;
    std::vector<std::string> tokens = tokenize(trimmed, tokenizeError);
    if (!tokenizeError.empty()) {
        return makeResponse(false, tokenizeError);
    }
    if (tokens.empty()) {
        return makeResponse(false, "Empty command.");
    }

    const std::string verb = lowerCopy(tokens[0]);
    if (verb == "help") {
        return executeHelp();
    }
    if (verb == "state") {
        const bool full = tokens.size() > 1 && lowerCopy(tokens[1]) == "full";
        if (tokens.size() > (full ? 2u : 1u)) {
            return makeResponse(false, "Usage: state or state full.");
        }
        return executeState(context, full);
    }
    if (verb == "catalog") {
        if (tokens.size() != 2) {
            return makeResponse(false, "Usage: catalog algorithms|sounds|effects.");
        }
        return executeCatalog(context, lowerCopy(tokens[1]));
    }
    if (verb == "meters") {
        if (tokens.size() != 1) {
            return makeResponse(false, "Usage: meters.");
        }
        return executeMeters(context);
    }
    if (verb == "listen") {
        double seconds = 8.0;
        if (tokens.size() > 2) {
            return makeResponse(false, "Usage: listen 8s.");
        }
        if (tokens.size() == 2 && !parseDurationSeconds(tokens[1], seconds)) {
            return makeResponse(false, "Listen duration must be a positive number of seconds.");
        }
        return executeListen(context, seconds);
    }
    if (verb == "play") {
        if (tokens.size() != 1) {
            return makeResponse(false, "Usage: play.");
        }
        return executePlay(context);
    }
    if (verb == "pause") {
        if (tokens.size() != 1) {
            return makeResponse(false, "Usage: pause.");
        }
        return executePause(context);
    }
    if (verb == "stop") {
        if (tokens.size() != 1) {
            return makeResponse(false, "Usage: stop.");
        }
        return executeStop(context);
    }
    if (verb == "tempo") {
        return executeTempo(context, tokens);
    }
    if (verb == "track") {
        return executeTrack(context, tokens);
    }
    if (verb == "master") {
        return executeMaster(context, tokens);
    }
    if (verb == "key") {
        return executeKey(context, tokens);
    }
    if (verb == "progression") {
        return executeProgression(context, tokens);
    }
    if (verb == "genre") {
        return executeGenre(context, tokens);
    }
    if (verb == "arrangement") {
        return executeArrangement(context, tokens);
    }
    if (verb == "packages") {
        return executePackages(context, tokens);
    }
    if (verb == "presets") {
        return executePresets(context, tokens);
    }
    if (verb == "record") {
        if (!context.recordFn) {
            return makeResponse(false, "Recording not available. Start with --agent-port or --mcp.");
        }
        std::string action = "status";
        std::string argsJson = "{}";
        if (tokens.size() > 1) {
            action = lowerCopy(tokens[1]);
            if (tokens.size() > 2) {
                auto args = std::make_unique<juce::DynamicObject>();
                for (size_t i = 2; i < tokens.size(); i += 2) {
                    if (i + 1 < tokens.size()) {
                        args->setProperty(juce::String(tokens[i]), juce::String(tokens[i + 1]));
                    }
                }
                argsJson = juce::JSON::toString(juce::var(args.release()), false).toStdString();
            }
        }
        std::string result = context.recordFn(action, argsJson);
        return makeResponse(true, result);
    }
    if (verb == "stream") {
        if (!context.streamFn) {
            return makeResponse(false, "Streaming not available. Start with --agent-port or --mcp.");
        }
        std::string action = "status";
        std::string argsJson = "{}";
        if (tokens.size() > 1) {
            action = lowerCopy(tokens[1]);
            if (tokens.size() > 2) {
                auto args = std::make_unique<juce::DynamicObject>();
                for (size_t i = 2; i < tokens.size(); i += 2) {
                    if (i + 1 < tokens.size()) {
                        args->setProperty(juce::String(tokens[i]), juce::String(tokens[i + 1]));
                    }
                }
                argsJson = juce::JSON::toString(juce::var(args.release()), false).toStdString();
            }
        }
        std::string result = context.streamFn(action, argsJson);
        return makeResponse(true, result);
    }
    if (verb == "p2p") {
        if (!context.p2pFn) {
            return makeResponse(false, "P2P bridge not available. Start with --agent-port.");
        }
        // p2p.peers is handled locally (doesn't need P2PClient)
        if (tokens.size() > 1 && lowerCopy(tokens[1]) == "peers") {
            auto* res = new juce::DynamicObject();
            res->setProperty("ok", true);
            res->setProperty("message", "Use --discover-peers flag. Peer list available via MCP tools.");
            return makeResponse(true, juce::JSON::toString(juce::var(res), false).toStdString());
        }
        // p2p.<tool_name> [args_as_json]
        std::string subCommand;
        std::string argsJson = "{}";
        if (tokens.size() > 1) {
            subCommand = tokens[1];
            // Reconstruct remaining tokens as JSON args
            if (tokens.size() > 2) {
                auto args = std::make_unique<juce::DynamicObject>();
                for (size_t i = 2; i < tokens.size(); i += 2) {
                    if (i + 1 < tokens.size()) {
                        args->setProperty(juce::String(tokens[i]), juce::String(tokens[i + 1]));
                    }
                }
                argsJson = juce::JSON::toString(juce::var(args.release()), false).toStdString();
            }
        }
        if (subCommand.empty()) {
            // p2p without subcommand: return status
            auto* res = new juce::DynamicObject();
            res->setProperty("ok", true);
            res->setProperty("message", "P2P bridge available. Commands: p2p.search, p2p.publish, p2p.download, p2p.verify, p2p.peers");
            return makeResponse(true, juce::JSON::toString(juce::var(res), false).toStdString());
        }
        std::string toolName = subCommand;
        std::string result = context.p2pFn(toolName, argsJson);
        return makeResponse(true, result);
    }
    if (verb == "swing") {
        return executeSwing(context, tokens);
    }
    if (verb == "humanize") {
        return executeHumanize(context, tokens);
    }
    if (verb == "project") {
        return executeProject(context, tokens);
    }

    return makeResponse(false, "Unknown command. Try help.");
}

} // namespace AgentCommand
